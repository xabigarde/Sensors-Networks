package org.egokituz.arduino2android.models;

import android.util.Log;

import org.egokituz.arduino2android.ArduinoThread;
import org.egokituz.arduino2android.models.exceptions.BadMessageFrameFormat;
import org.egokituz.arduino2android.models.exceptions.IncorrectETXbyte;
import org.egokituz.arduino2android.models.exceptions.UnknownMessageID;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.CRC32;
import java.util.zip.Checksum;


/**
 * @author Xabier Gardeazabal
 *
 * This class builds a message according to the specified message format
 * and retrieves the different fields 
 */
public class ArduinoMessage{
	private static final String TAG = "MessageReading";
	
	public static final int STX = 0x02; // Start of Text flag 
	public static final int MSGID_PING = 0x26; // Message ID: ping type
	public static final int MSGID_DATA = 0x27; // Message ID: Data type
	public static  final int ETX = 0x03; // End of Text flag

	public String devName = "";
	public String devMAC = "";
	public long timestamp;

	private byte stx;
	private byte msgId;
	private byte frameSeqNum;
	private byte dlcBytes[] = new byte[4];
	private int dlc;
	private String payload;
	private byte crc_bytes[] = new byte[8];
	private byte etx;
	private long crc;
	//private ByteBuffer auxBuffer = ByteBuffer.allocate(Long.SIZE);
	
	public ArduinoMessage(){
		
	}

	/**
	 * Constructs an ArduinoMessage instance from the buffer provided
	 * @param buffer
	 * @throws BadMessageFrameFormat
	 */
	public ArduinoMessage(byte[] buffer) throws BadMessageFrameFormat {
		int bufferIndex = 0;

		try {
			stx 				= buffer[bufferIndex++];
			if(stx != STX)
				throw new BadMessageFrameFormat();
			msgId 				= buffer[bufferIndex++];
			if(msgId != MSGID_DATA && msgId != MSGID_PING)
				throw new UnknownMessageID("Received "+msgId+". Expected MSGID_DATA or _PING");
			frameSeqNum			= buffer[bufferIndex++];
			dlcBytes[0]			= buffer[bufferIndex++];
			dlcBytes[1]			= buffer[bufferIndex++];
			dlcBytes[2]			= buffer[bufferIndex++];
			dlcBytes[3]			= buffer[bufferIndex++];
			
			ByteBuffer dlcBuffer = ByteBuffer.wrap(dlcBytes);
			dlcBuffer.order(ByteOrder.BIG_ENDIAN);
			dlc = dlcBuffer.getInt();
			
			payload = new String(buffer, bufferIndex, dlc);
			bufferIndex+=dlc;
			crc_bytes [0]= buffer[bufferIndex++];
			crc_bytes [1]= buffer[bufferIndex++];
			crc_bytes [2]= buffer[bufferIndex++];
			crc_bytes [3]= buffer[bufferIndex++];
			etx = buffer[bufferIndex];
			if(etx != ETX)
				throw new IncorrectETXbyte();

			ByteBuffer auxBuffer = ByteBuffer.wrap(crc_bytes);
			auxBuffer.order(ByteOrder.LITTLE_ENDIAN);
			crc = auxBuffer.getLong();

			long checksum = getChecksum(payload.getBytes());

			if(checksum != crc){
				Log.e(TAG, "Payload contains erros: ecpected: "+crc+" calc."+checksum);
				throw new BadMessageFrameFormat();
			}
		} catch (Exception e) {
			/* An exception should only happen if the buffer is too short and we walk off the end of the bytes.
			 * Because of the way we read the bytes from the device this should never happen, but just in case
			 * we'll catch the exception */
			Log.d(TAG, "Failure building MessageReading from byte buffer, probably an incopmplete or corrupted buffer");
			//e.printStackTrace();
			throw new BadMessageFrameFormat();

		}
	}

	/**
	 * Constructs an ArduinoMessage instance with the buffer provided
	 * @param devName identifier of the target device it is intended to be sent.
	 * @param auxBuff
	 * @throws BadMessageFrameFormat
	 */
	public ArduinoMessage(String devName, byte[] auxBuff) throws BadMessageFrameFormat {
		this(auxBuff);
		this.devName = devName;
	}
	
	/**
	 * Constructs an instance of ArduinoMessage, of type {ArduinoMessage.MSGID_PING}, and with the sequence number provided
	 * @param sequenceNumber
	 * @return
	 */
	public List<Byte> pingMessage(int sequenceNumber){
		String msg = "p";
		int size = msg.length();
		//byte[] outBuffer = new byte[16];
		List<Byte> outBuffer = new ArrayList<Byte>();
		outBuffer.add((byte) STX);
		outBuffer.add((byte) MSGID_PING);
		outBuffer.add((byte) sequenceNumber);
		//outBuffer.add((byte) size);
		for( byte b : ByteBuffer.allocate(4).putInt(size).array())
			outBuffer.add((byte) b);
		for(byte b : msg.getBytes())
			outBuffer.add((byte) b);
		long CRC = getChecksum(msg.getBytes());
		for( byte b : longToBytes(CRC))
			outBuffer.add((byte) b);
		outBuffer.add((byte) ETX);

		return outBuffer;
	}

    /**
     * Constructs a new ArduinoMessage in a byte list format, with the sequence number provided
     * @param msgType The type of the message (MSGID_PING or MSGID_DATA)
     * @param sequenceNumber
     * @param msg The data/message to be inserted as payload
     * @return
     */
    public static List<Byte> constructNewMessage(int msgType, int sequenceNumber, String msg){
        int size = msg.length();
        //byte[] outBuffer = new byte[16];
        List<Byte> outBuffer = new ArrayList<Byte>();
        outBuffer.add((byte) STX); // start of message byte
        outBuffer.add((byte) MSGID_PING); // message ID byte
        outBuffer.add((byte) sequenceNumber); // add sequence number byte
        for( byte b : ByteBuffer.allocate(4).putInt(size).array())
            outBuffer.add((byte) b); // 4 bytes to define the payload size
        for(byte b : msg.getBytes())
            outBuffer.add((byte) b); //the payload bytes
        long CRC = getChecksum(msg.getBytes());
        for( byte b : longToBytes(CRC))
            outBuffer.add((byte) b); //the CRC bytes
        outBuffer.add((byte) ETX); // the ETX byte

        return outBuffer;
    }

	/**
	 * @return Returns the number of bytes of this instance of ArduinoMessage
	 */
	public int size(){
		int result = 1+1+1+4+dlc+4+1; //STX + ID + Seq.Num + 4*DLC + payload size + 4*CRC + ETX
		return result;
	}

	/**
	 * @return The payload of this instance of {@link ArduinoThread}
	 */
	public String getPayload() {
		return payload;
	}

	/**
	 * The possible frame sequence numbers range from 1 to 99 (inclussive)
	 * @return Returns the frame sequence number of this instance of {@link ArduinoMessage}. 
	 */
	public int getFrameNum(){
		return frameSeqNum;
	}
	
	/**
	 * The possible message IDs are {ArduinoMessage.MSGID_PING} = 0x26 and {ArduinoMessage.MSGID_DATA} = 0x27
	 * @return The message ID of this instance
	 */
	public int getMessageID(){
		return msgId;
	}

	/**
	 * Calculates the CRC (Cyclic Redundancy Check) checksum value of the given bytes
	 * according to the CRC32 algorithm.
	 * @param bytes 
	 * @return The CRC32 checksum
	 */
	private static long getChecksum(byte bytes[]){

		Checksum checksum = new CRC32();

		// update the current checksum with the specified array of bytes
		checksum.update(bytes, 0, bytes.length);

		// get the current checksum value
		long checksumValue = checksum.getValue();

		return checksumValue;
	}
	
	/**
	 * 
	 * @param l
	 * @return The provided long in a byte array ordered by the {ByteOrder.LITTLE_ENDIAN}.
	 */
	public static byte[] longToBytes(long l) {
		ByteBuffer buffer = ByteBuffer.allocate(8);
		buffer.order(ByteOrder.LITTLE_ENDIAN);
		buffer.putLong(l);

		byte[] result = new byte[4];
		byte[] b = buffer.array();
		for(int i=0; i<4; i++)
			result[i] = b[i];
		return result;
	}
	
	public static long bytesToLong(byte[] bytes) {
	    ByteBuffer buffer = ByteBuffer.allocate(4);
	    buffer.put(bytes);
	    buffer.flip();//need flip 
	    return buffer.getLong();
	}
	
	
}