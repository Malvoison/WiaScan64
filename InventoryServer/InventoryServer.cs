using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.Odbc;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows;
using System.Xml;
using System.Xml.Linq;
using System.Xml.XPath;

namespace InventoryServer
{

    public class InventoryServer
    {

        #region Constants

        [StructLayout(LayoutKind.Explicit, Size = 8)]
        private struct CHANNEL_PDU_HEADER
        {
            [FieldOffset(0)]
            public UInt32 length;
            [FieldOffset(4)]
            public UInt32 flags;
        }

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

        private const string CHANNELNAME_SERVERCOMMAND = "SRVCMD";
        private const string CHANNELNAME_SERVERDATA = "SRVDAT";
        private const string CHANNELNAME_SERVEROUTPUT = "SRVOUT";

        private const uint INFINITE = 0xFFFFFFFF;

        private const int VC_TIMEOUT = 30000;
        private const int MAX_BUFFER_SIZE = 1600;
        private const int MAX_IMAGE_CHUNK = 1592;

        private const int WM_USER = 0x0400;
        private const int WM_APP = 0x8000;

        private const int PM_INIT = WM_APP + 0x0001;
        private const int PM_SENDDATA = WM_APP + 0x0002;
        private const int PM_PING = WM_APP + 0x0100;
        private const int PM_DATASENT = WM_APP + 0x0200;

        #endregion

        #region DLL Imports

        [DllImport("wtsapi32.dll", CharSet = CharSet.Ansi)]
        private static extern IntPtr WTSVirtualChannelOpen(IntPtr hServer,
            UInt32 SessionId, string pVirtualName);

        [DllImport("wtsapi32.dll")]
        private static extern Int32 WTSVirtualChannelClose(IntPtr hChannelHandle);

        [DllImport("wtsapi32.dll", SetLastError = true)]
        private static extern bool WTSVirtualChannelRead(IntPtr hChannelHandle,
            UInt32 TimeOut, IntPtr Buffer, UInt32 BufferSize, out UInt32 pBytesRead);

        [DllImport("wtsapi32.dll")]
        private static extern bool WTSVirtualChannelWrite(IntPtr hChannelHandle,
            byte[] Buffer, UInt32 Length, out UInt32 pBytesWritten);

        [DllImport("wtsapi32.dll")]
        private static extern int WTSVirtualChannelQuery(IntPtr hChannelHandle,
            int WtsVirtualClass, out IntPtr ppBuffer, out uint pBytesReturned);

        #endregion

        #region Events

        public delegate void DataReceivedEventHandler(object sender, DataReceivedEventArgs args);

        [Serializable]
        public class DataReceivedEventArgs : EventArgs
        {
            DataSet _resultData;
            int _rowsAffected;
            string _errorInfo;

            public DataReceivedEventArgs(string sErrors)
                : this(new DataSet(), sErrors)
            {
            }

            public DataReceivedEventArgs(DataSet dsResults, int nRowsAffected, string sErrors)
            {
                _rowsAffected = nRowsAffected;
                _resultData = dsResults;
                _errorInfo = sErrors;
            }

            public DataReceivedEventArgs(DataSet dsResults, string sErrors)
                : this(dsResults, 0, sErrors)
            {
            }

            public DataReceivedEventArgs(int nRowsAffected, string sErrors)
                : this(new DataSet(), nRowsAffected, sErrors)
            {
            }

            public DataSet ResultData
            {
                get { return _resultData; }
            }

            public int RowsAffected
            {
                get { return _rowsAffected; }
            }

            public string ErrorInfo
            {
                get { return _errorInfo; }
            }
        }

        public event DataReceivedEventHandler DataReceivedEvent;

        public void RaiseDataReceived(DataReceivedEventArgs args)
        {
            try
            {
                if (DataReceivedEvent != null)
                    DataReceivedEvent(this, args);
            }
            catch (Exception e)
            {
                Trace.WriteLine(e.Message);
            }
        }

        #endregion

        private static Mutex singleMutex = new Mutex();
        private XDocument document = XDocument.Parse("<ROOT/>");
        private static InventoryServer _instance;
        private IntPtr hChannelCommandHandle = IntPtr.Zero;
        private IntPtr hChannelDataHandle = IntPtr.Zero;
        private IntPtr hChannelOutputHandle = IntPtr.Zero;

        private BlockingCollection<uint> _bcServerCommand = new BlockingCollection<uint>(new ConcurrentQueue<uint>());
        private BlockingCollection<byte[]> _bcServerData = new BlockingCollection<byte[]>(new ConcurrentQueue<byte[]>());

        private InventoryServer()
        {
        }

        public static InventoryServer GetInstance()
        {
            if (_instance == null)
            {
                _instance = new InventoryServer();
                _instance.Initialize();
            }
            return _instance;
        }

        public void ExecuteDataSet(string sql)
        {
            XElement query = new XElement("QUERY");
            query.Add(new XElement("SELECT", sql));
            query.Add(new XElement("ROWCOUNT"));
            query.Add(new XElement("ERROR"));
            query.Add(new XElement("DATA"));

            document.Root.Add(query);

            SendData(document.ToString());
        }

        public void ExecuteNonQuery(string sql)
        {
            XElement query = new XElement("QUERY");

            query.Add(new XElement("DML", sql));
            query.Add(new XElement("ROWCOUNT"));
            query.Add(new XElement("ERROR"));
            document.Root.Add(query);

            SendData(document.ToString());
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

        private void Initialize()
        {
            OpenVirtualChannelsAndMonitors();
        }

        private void OpenVirtualChannelsAndMonitors()
        {
            //  hServerCommandChannelHandle
            hChannelCommandHandle = WTSVirtualChannelOpen(IntPtr.Zero, INFINITE, CHANNELNAME_SERVERCOMMAND);
            if (hChannelCommandHandle != IntPtr.Zero)
            {
                Thread thread = new Thread(new ThreadStart(ServerCommandChannelMonitor));
                thread.IsBackground = true;
                thread.Start();

                SendServerCommand(PM_INIT);
            }
            else
                Trace.WriteLine("WTSVirtualChannelOpen failed for channel: " + CHANNELNAME_SERVERCOMMAND);

            hChannelDataHandle = WTSVirtualChannelOpen(IntPtr.Zero, INFINITE, CHANNELNAME_SERVERDATA);
            if (hChannelDataHandle != IntPtr.Zero)
            {
                Thread thread = new Thread(new ThreadStart(ServerDataChannelMonitor));
                thread.IsBackground = true;
                thread.Start();
            }

            hChannelOutputHandle = WTSVirtualChannelOpen(IntPtr.Zero, INFINITE, CHANNELNAME_SERVEROUTPUT);
            if (hChannelOutputHandle != IntPtr.Zero)
            {
                Thread thread = new Thread(new ThreadStart(Listener));
                thread.IsBackground = true;
                thread.Start();
            }
        }

        private void Listener()
        {
            string totalMessage = "";
            while (true)
            {
                try
                {
                    IntPtr PduBuffer = Marshal.AllocHGlobal(1600);
                    byte[] MsgBuffer = null;
                    UInt32 BytesRead = 1;
                    UInt32 TotalMsgBytesRead = 0;
                    CHANNEL_PDU_HEADER pdu = new CHANNEL_PDU_HEADER();
                    while (WTSVirtualChannelRead(hChannelOutputHandle, 5000, PduBuffer, CHANNEL_CHUNK_LENGTH, out BytesRead))
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

                        TotalMsgBytesRead += BytesRead;

                        //  done with this message
                        if ((pdu.flags & CHANNEL_FLAG_LAST) != 0)
                        {
                            string resultXml = System.Text.Encoding.Unicode.GetString(MsgBuffer);
                            Trace.WriteLine(resultXml);
                            ParseXmlDocument(resultXml);
                            //  Release the MsgBuffer
                            totalMessage += resultXml;
                            MsgBuffer = null;
                            TotalMsgBytesRead = 0;
                        }
                    }
                }
                catch (Exception ex)
                {
                    Trace.WriteLine(ex.Message);
                    MessageBox.Show(string.Format("{0}: {1}", ex.StackTrace, ex.Message));
                    break;
                }
            }
        }

        private void SendData(string str)
        {
            _bcServerData.Add(Encoding.Unicode.GetBytes(str + '\0'));
        }

        private void ServerCommandChannelMonitor()
        {
            foreach (var item in _bcServerCommand.GetConsumingEnumerable())
            {
                byte[] InBuffer = new byte[4];
                uint[] nCmd = new uint[1];
                nCmd[0] = item;
                Buffer.BlockCopy(nCmd, 0, InBuffer, 0, 4);
                UInt32 BytesWritten = 0;

                bool bResult = WTSVirtualChannelWrite(hChannelCommandHandle, InBuffer, (uint)InBuffer.Length, out BytesWritten);
                if (!bResult)
                {
                    Console.WriteLine("WTSVirtualChannelWrite failed for PM_INIT");
                    Trace.WriteLine("WTSVirtualChannelWrite failed for PM_INIT");
                }
            }
        }

        private void ServerDataChannelMonitor()
        {
            foreach (byte[] item in _bcServerData.GetConsumingEnumerable())
            {
                singleMutex.WaitOne();

                uint bytesResult;
                bool bResult = WTSVirtualChannelWrite(hChannelDataHandle, item, (uint)item.Length, out bytesResult);

                singleMutex.ReleaseMutex();
            }
        }

        private void SendDBResults(string resultXml)
        {
            try
            {
                RaiseDataReceived(ParseXmlDocument(resultXml));
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        private DataReceivedEventArgs ParseXmlDocument(string resultXml)
        {
            XDocument document = XDocument.Parse(resultXml);
            DataSet set = new DataSet();
            int rowAffected = 0;
            string errorString = "";

            XElement element = document.Root.XPathSelectElement("QUERY");

            XElement data = element.XPathSelectElement("NewDataSet");
            if (data != null)
            {
                using (MemoryStream ms = new MemoryStream(Encoding.Default.GetBytes(data.ToString())))
                {
                    set.ReadXml(ms);
                }
            }

            XElement rowCountElement = element.XPathSelectElement("ROWCOUNT");
            if (rowCountElement != null)
            {
                Int32.TryParse(rowCountElement.Value, out rowAffected);
            }

            XElement errorElement = element.XPathSelectElement("ERROR");
            if (errorElement != null)
            {
                errorString = errorElement.Value;
            }

            return new DataReceivedEventArgs(set, rowAffected, errorString);
        }
    }
}
