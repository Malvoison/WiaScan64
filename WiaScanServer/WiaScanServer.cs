using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Runtime.InteropServices;
using System.Threading;
using System.Diagnostics;
using Microsoft.Win32.SafeHandles;
using System.IO;
using System.Collections.Concurrent;
using System.ComponentModel;
using System.Collections;

namespace WiaScanServer
{
    public delegate void ImageCallbackDelegate(byte[] ms, int nDestLen, int nSequence);
    public delegate void ImageCallbackEventHandler(object sender, ImageCallbackEventArgs ica);

    [Serializable]
    public class ImageCallbackEventArgs : EventArgs
    {
        int _imageCount;

        public ImageCallbackEventArgs(int nImageCount)
        {
            _imageCount = nImageCount;
        }

        public int ImageCount
        {
            get { return _imageCount; }
        }
    }

    public class WiaScanServer
    {
        [StructLayout(LayoutKind.Explicit, Size=8)]
        private struct CHANNEL_PDU_HEADER
        {
            [FieldOffset(0)]
            public UInt32 length;
            [FieldOffset(4)]
            public UInt32 flags;
        }

        #region Events

        public event ImageCallbackEventHandler BeforeImageCallback;
        public event ImageCallbackEventHandler AfterImageCallback;
        public event EventHandler PicsReady;

        #endregion

        #region Constants

        /****************************************************************************/
        /* Header flags (also passed to VirtualChannelOpenEventFn)                  */
        /****************************************************************************/
        private const uint CHANNEL_FLAG_FIRST = 0x01;
        private const uint CHANNEL_FLAG_LAST = 0x02;
        private const uint CHANNEL_FLAG_ONLY = (CHANNEL_FLAG_FIRST | CHANNEL_FLAG_LAST);
        private const uint CHANNEL_FLAG_MIDDLE = 0;

        private const uint CHANNEL_CHUNK_LENGTH = 1600;
        private const uint CHANNEL_BUFFER_SIZE = 65535;
        private const int PDU_HEADER_SIZE = 8;

        private const uint CHANNEL_PDU_LENGTH = (CHANNEL_CHUNK_LENGTH + PDU_HEADER_SIZE);
        
        public const string CHANNELNAME_SERVERCOMMAND = "SRVCMD";
        public const string CHANNELNAME_CLIENTINFO = "CLNTNOT";
        public const string CHANNELNAME_IMAGEXFER1 = "XFER1";
        public const string CHANNELNAME_IMAGEXFER2 = "XFER2";
        public const string CHANNELNAME_IMAGEXFER3 = "XFER3";
        public const string CHANNELNAME_IMAGEXFER4 = "XFER4";
        private const int IMAGEXFER1 = 0;
        private const int IMAGEXFER2 = 1;
        private const int IMAGEXFER3 = 2;
        private const int IMAGEXFER4 = 3;

        private const uint INFINITE = 0xFFFFFFFF;

        private const int SCAN_VER = 4;

        private const int VC_TIMEOUT = 30000;
        private const int MAX_BUFFER_SIZE = 1600;
        private const int MAX_IMAGE_CHUNK = 1592;

        private const int WM_USER = 0x0400;
        private const int WM_APP = 0x8000;

        private const int PM_INIT = WM_APP + 0x0004;
        private const int PM_GETDEFAULTSOURCE = WM_APP + 0x0005;
        private const int PM_SELECTSRC = WM_APP + 0x0006;
        private const int PM_ENUMSRCS = WM_APP + 0x0007;
        private const int PM_SETSELECTEDSOURCE = WM_APP + 0x0008;
        private const int PM_SETPIXELTYPE = WM_APP + 0x0009;
        private const int PM_ENUMPIXELTYPES = WM_APP + 0x000A;
        private const int PM_SETBITDEPTH = WM_APP + 0x000B;
        private const int PM_ENUMBITDEPTHS = WM_APP + 0x000C;
        private const int PM_SETRESOLUTION = WM_APP + 0x000D;
        private const int PM_GETRESOLUTIONRANGE = WM_APP + 0x000E;
        private const int PM_ENUMRESOLUTIONS = WM_APP + 0x000F;
        private const int PM_ACQUIRE = WM_APP + 0x0010;
        private const int PM_TRANSFERPICTURES = WM_APP + 0x0011;
        private const int PM_RELEASE = WM_APP + 0x0012;
        private const int PM_SHOWUI = WM_APP + 0x0013;
        private const int PM_ENUMCAPS = WM_APP + 0x0014;
        private const int PM_ENUMPAGESIZES = WM_APP + 0x0015;
        private const int PM_SETPAGESIZE = WM_APP + 0x0016;
        private const int PM_SETFEEDERENABLED = WM_APP + 0x0017;
        private const int PM_SETDUPLEX = WM_APP + 0x0018;
        public const int PM_PING = WM_APP + 0x0100;
        private const int PM_PICSTART = WM_APP + 0x0200;
        private const int PM_PICDATA = WM_APP + 0x0201;
        private const int PM_PICDONE = WM_APP + 0x0202;
        private const int PM_TRANSFERDONE = WM_APP + 0x0203;
        private const int PM_TRANSFERREADY = WM_APP + 0x0204;
        private const int PM_FNCOMPLETE = WM_APP + 0x0205;
        private const int PM_ACQUIREDIRECT = WM_APP + 0x0206;
        private const int PM_TRANSFERDIRECTPICTURES = WM_APP + 0x0207;
        private const int PM_TRANSFERDIRECTREADY = WM_APP + 0x0208;
        private const int PM_TRANSFERDIRECTDONE = WM_APP + 0x0209;
        private const int PM_ACKPICDATA = WM_APP + 0x020A;
        private const int PM_VERCHECK = WM_APP + 0x0300;
        private const int PM_LOCALLAUNCH = WM_APP + 0x0401;
        public const int PM_SETPREFERENCES = WM_APP + 0x0402;

        public const int PM_GETVERSION = WM_APP + 0x0501;
        public const int PM_UDPATEVERSION = WM_APP + 0x0502;

        #endregion

        #region DLL Imports

        [DllImport("wtsapi32.dll", CharSet = CharSet.Ansi)]
        private static extern IntPtr WTSVirtualChannelOpen(IntPtr hServer,
            UInt32 SessionId, string pVirtualName);

        [DllImport("wtsapi32.dll")]
        private static extern Int32 WTSVirtualChannelClose(IntPtr hChannelHandle);

        [DllImport("wtsapi32.dll", SetLastError=true)]
        private static extern bool WTSVirtualChannelRead(IntPtr hChannelHandle,
            UInt32 TimeOut, IntPtr Buffer, UInt32 BufferSize, out UInt32 pBytesRead);

        [DllImport("wtsapi32.dll")]
        private static extern bool WTSVirtualChannelWrite(IntPtr hChannelHandle,
            byte[] Buffer, UInt32 Length, out UInt32 pBytesWritten);

        [DllImport("wtsapi32.dll")]
        private static extern int WTSVirtualChannelQuery(IntPtr hChannelHandle,
            int WtsVirtualClass, out IntPtr ppBuffer, out uint pBytesReturned);

        #endregion

        #region Public Properties

        public ArrayList ImageList
        {
            get { return alImageList; }
            set { alImageList = ArrayList.Synchronized(value); }
        }

        public ImageCallbackDelegate ImageCallback
        {
            get { return icd; }
            set { icd = value; }
        }

        #endregion

        #region Private Fields

        private static WiaScanServer _instance;
        private IntPtr hServerCommandChannelHandle = IntPtr.Zero;
        private IntPtr hClientInfoChannelHandle = IntPtr.Zero;
        private IntPtr hImageXfer1ChannelHandle = IntPtr.Zero;
        private IntPtr hImageXfer2ChannelHandle = IntPtr.Zero;
        private IntPtr hImageXfer3ChannelHandle = IntPtr.Zero;
        private IntPtr hImageXfer4ChannelHandle = IntPtr.Zero;
        private IntPtr[] ImageXferChannelHandle = new IntPtr[4];

        private BlockingCollection<uint> _bcServerCommand = new BlockingCollection<uint>(new ConcurrentQueue<uint>());
        private ArrayList alImageList = null;
        private ImageCallbackDelegate icd = null;

        #endregion

        public WiaScanServer()
        {
        }

        public static WiaScanServer GetInstance()
        {
            if (_instance == null)
            {
                _instance = new WiaScanServer();
                _instance.Initialize();
            }

            return _instance;
        }

        #region Private Methods

        private Thread threadServerCommand;
        private Thread threadClientInfo;
        private Thread threadImageXfer1;
        private Thread threadImageXfer2;
        private Thread threadImageXfer3;
        private Thread threadImageXfer4;

        private void OpenVirtualChannelsAndMonitors()
        {
            //  hServerCommandChannelHandle
            hServerCommandChannelHandle = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_SERVERCOMMAND);
            if (hServerCommandChannelHandle != IntPtr.Zero)
            {
                threadServerCommand = new Thread(new ThreadStart(ServerCommandChannelMonitor));
                threadServerCommand.IsBackground = true;
                threadServerCommand.Start();
                SendServerCommand(PM_INIT);           
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_SERVERCOMMAND);
            }

            //  hClientInfoChannelHandle
            hClientInfoChannelHandle = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_CLIENTINFO);
            if (hClientInfoChannelHandle != IntPtr.Zero)
            {
                threadClientInfo = new Thread(new ThreadStart(ClientInfoChannelMonitor));
                threadClientInfo.IsBackground = true;
                threadClientInfo.Start();                
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_CLIENTINFO);
            }

            //  hImageXfer1ChannelHandle
            ImageXferChannelHandle[IMAGEXFER1] = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_IMAGEXFER1);
            if (ImageXferChannelHandle[IMAGEXFER1] != IntPtr.Zero)
            {
                threadImageXfer1 = new Thread(new ParameterizedThreadStart(ImageTransfer1ChannelMonitor));
                threadImageXfer1.IsBackground = true;
                threadImageXfer1.Start(IMAGEXFER1);                
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_IMAGEXFER1);
            }

            //  hImageXfer2ChannelHandle
            ImageXferChannelHandle[IMAGEXFER2] = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_IMAGEXFER2);
            if (ImageXferChannelHandle[IMAGEXFER2] != IntPtr.Zero)
            {
                threadImageXfer2 = new Thread(new ParameterizedThreadStart(ImageTransfer2ChannelMonitor));
                threadImageXfer2.IsBackground = true;
                threadImageXfer2.Start(IMAGEXFER2);                
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_IMAGEXFER2);
            }

            //  hImageXfer3ChannelHandle
            ImageXferChannelHandle[IMAGEXFER3] = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_IMAGEXFER3);
            if (ImageXferChannelHandle[IMAGEXFER3] != IntPtr.Zero)
            {
                threadImageXfer3 = new Thread(new ParameterizedThreadStart(ImageTransfer3ChannelMonitor));
                threadImageXfer3.IsBackground = true;
                threadImageXfer3.Start(IMAGEXFER3);                
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_IMAGEXFER3);
            }

            //  hImageXfer4ChannelHandle
            ImageXferChannelHandle[IMAGEXFER4] = WTSVirtualChannelOpen(IntPtr.Zero, 0xFFFFFFFF, CHANNELNAME_IMAGEXFER4);
            if (ImageXferChannelHandle[IMAGEXFER4] != IntPtr.Zero)
            {
                threadImageXfer4 = new Thread(new ParameterizedThreadStart(ImageTransfer4ChannelMonitor));
                threadImageXfer4.IsBackground = true;
                threadImageXfer4.Start(IMAGEXFER4);
            }
            else
            {
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_IMAGEXFER4);
            }

        }

        private void CloseVirtualChannelsAndMonitors()
        {
            threadServerCommand.Abort();
            threadClientInfo.Abort();
            threadImageXfer1.Abort();
            threadImageXfer2.Abort();
            threadImageXfer3.Abort();
            threadImageXfer4.Abort();

            WTSVirtualChannelClose(hServerCommandChannelHandle);
            WTSVirtualChannelClose(hClientInfoChannelHandle);
            WTSVirtualChannelClose(hImageXfer1ChannelHandle);
            WTSVirtualChannelClose(hImageXfer2ChannelHandle);
            WTSVirtualChannelClose(hImageXfer3ChannelHandle);
            WTSVirtualChannelClose(hImageXfer4ChannelHandle);
        }

        public void Initialize()
        {
            OpenVirtualChannelsAndMonitors();
        }

        public void Release()
        {
            CloseVirtualChannelsAndMonitors();
        }

        #endregion

        #region Server Command Channel - Read/Write Channel

        private void ServerCommandChannelMonitor()
        {
            foreach (var item in _bcServerCommand.GetConsumingEnumerable())
            {
                byte[] InBuffer = new byte[4];
                uint[] nCmd = new uint[1];
                nCmd[0] = item;
                Buffer.BlockCopy(nCmd, 0, InBuffer, 0, 4);
                UInt32 BytesWritten = 0;

                bool bResult = WTSVirtualChannelWrite(hServerCommandChannelHandle, InBuffer, (uint)InBuffer.Length, out BytesWritten);
                if (!bResult)
                {
                    Trace.WriteLine("WTSVirtualChannelWrite failed for PM_PING");
                }
            }
        }

        public void SendServerCommand()
        {
            try
            {
                _bcServerCommand.Add((uint)PM_PING);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        public void SendServerCommand(uint cmd)
        {
            try
            {
                _bcServerCommand.Add(cmd);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        public void Scan()
        {
            _bcServerCommand.Add((uint)PM_ACQUIRE);
        }

        #endregion


        #region Client Info Channel

        private void ClientInfoChannelMonitor()
        {
            Trace.WriteLine("Starting ClientInfoChannelMonitor");

            IntPtr PduBuffer = Marshal.AllocHGlobal((int)CHANNEL_CHUNK_LENGTH);            
            byte[] MsgBuffer = null;
            UInt32 BytesRead = 0;
            UInt32 TotalMsgBytesRead = 0;
            CHANNEL_PDU_HEADER pdu = new CHANNEL_PDU_HEADER();
            while (WTSVirtualChannelRead(hClientInfoChannelHandle, INFINITE, PduBuffer, CHANNEL_CHUNK_LENGTH, out BytesRead))
            {
                //  copy the PDU header
                byte[] arrBuffer = new byte[BytesRead];
                Marshal.Copy(PduBuffer, arrBuffer, 0, (int)BytesRead);
                uint[] arrPdu = { 0, 0 };
                Buffer.BlockCopy(arrBuffer, 0, arrPdu, 0, 8);
                pdu.length = arrPdu[0];
                pdu.flags = arrPdu[1];
                
                //  resize the buffer for the new data
                Array.Resize(ref MsgBuffer, (int)((MsgBuffer == null) ? pdu.length : MsgBuffer.Length + pdu.length));
                Array.Copy(arrBuffer, PDU_HEADER_SIZE, MsgBuffer, TotalMsgBytesRead, pdu.length);

                TotalMsgBytesRead += pdu.length;
                
                //  done with this message
                if ((pdu.flags & CHANNEL_FLAG_LAST) != 0)
                {
                    string sClientInfo = System.Text.Encoding.Unicode.GetString(MsgBuffer);
                    Trace.WriteLine(sClientInfo);
                    //  Release the MsgBuffer
                    MsgBuffer = null;
                    TotalMsgBytesRead = 0;
                }
            }
        }

        #endregion

        private void ImageTransferHelper(int channel)
        {
            try
            {
                Trace.WriteLine("Starting Image Transfer Channel Monitor for Channel #" + channel.ToString());
                IntPtr PduBuffer = Marshal.AllocHGlobal((int)CHANNEL_PDU_LENGTH);
                byte[] MsgBuffer = null;
                UInt32 BytesRead = 0;
                CHANNEL_PDU_HEADER pdu = new CHANNEL_PDU_HEADER();
                DateTime dtStart = DateTime.MinValue;
                DateTime dtFinish;

                while (true)
                {
                    MemoryStream msBuffer = new MemoryStream();
                    do
                    {
                        bool bResult = WTSVirtualChannelRead(ImageXferChannelHandle[channel], INFINITE, PduBuffer, CHANNEL_PDU_LENGTH, out BytesRead);
                        if (bResult && (BytesRead > 0))
                        {
                            if (dtStart == DateTime.MinValue)
                                dtStart = DateTime.Now;
                            //  copy the PDU header
                            byte[] arrBuffer = new byte[BytesRead];
                            Marshal.Copy(PduBuffer, arrBuffer, 0, (int)BytesRead);
                            uint[] arrPdu = { 0, 0 };
                            Buffer.BlockCopy(arrBuffer, 0, arrPdu, 0, 8);
                            pdu.length = arrPdu[0];
                            pdu.flags = arrPdu[1];

                            msBuffer.Write(arrBuffer, 8, (int)(BytesRead - PDU_HEADER_SIZE));

                            //  done with this message
                            if ((pdu.flags & CHANNEL_FLAG_LAST) != 0)
                            {
                                MsgBuffer = msBuffer.ToArray();
                                //  OK, first 4 bytes are the sequence number
                                int[] nSequenceNumber = { 0 };
                                Buffer.BlockCopy(MsgBuffer, 0, nSequenceNumber, 0, 4);

                                dtFinish = DateTime.Now;
                                TimeSpan ts = dtFinish - dtStart;
                                Trace.WriteLine("Total transfer time: " + ts.TotalMilliseconds + " ms");

                                byte[] arrFile = new byte[MsgBuffer.Length - 4];
                                Array.Copy(MsgBuffer, 4, arrFile, 0, MsgBuffer.Length - 4);

                                if (BeforeImageCallback != null)
                                {
                                    ImageCallbackEventHandler eh = null;
                                    foreach (Delegate del in BeforeImageCallback.GetInvocationList())
                                    {
                                        try
                                        {
                                            eh = (ImageCallbackEventHandler)del;
                                            eh(this, new ImageCallbackEventArgs(1));
                                        }
                                        catch (Exception ex)
                                        {
                                            BeforeImageCallback -= eh;
                                        }
                                    }                                    
                                }

                                if (ImageCallback != null)
                                {
                                    ImageCallback(arrFile, arrFile.Length, nSequenceNumber[0]);
                                }

                                if (AfterImageCallback != null)
                                {
                                    ImageCallbackEventHandler eh = null;
                                    foreach (Delegate del in AfterImageCallback.GetInvocationList())
                                    {
                                        try
                                        {
                                            eh = (ImageCallbackEventHandler)del;
                                            eh(this, new ImageCallbackEventArgs(0));
                                        }
                                        catch (Exception ex)
                                        {
                                            AfterImageCallback -= eh;
                                        }
                                    }                                    
                                }

                                //  Release the MsgBuffer
                                MsgBuffer = null;
                                dtStart = DateTime.MinValue;
                            }
                        }
                    } while (BytesRead == CHANNEL_PDU_LENGTH);

                    msBuffer.Dispose();
                    msBuffer = null;
                }

                Marshal.FreeHGlobal(PduBuffer);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
            }
        }

        #region Image Transfer 1 Channel - Read Channel

        private void ImageTransfer1ChannelMonitor(object param)
        {
            ImageTransferHelper((int)param);
        }

        #endregion

        #region Image Transfer 2 Channel - Read Channel

        private void ImageTransfer2ChannelMonitor(object param)
        {
            ImageTransferHelper((int)param);
        }

        #endregion

        #region Image Transfer 3 Channel - Read Channel

        private void ImageTransfer3ChannelMonitor(object param)
        {
            ImageTransferHelper((int)param);
        }

        #endregion

        #region Image Transfer 4 Channel - Read Channel

        private void ImageTransfer4ChannelMonitor(object param)
        {
            ImageTransferHelper((int)param);
        }

        #endregion
    }
}
