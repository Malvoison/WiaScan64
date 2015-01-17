using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using System.Xml.XPath;
using System.Xml.Linq;
using System.Data.Odbc;
using System.Xml;
using System.Text;
using System.Data.Common;
using System.Runtime.InteropServices;
using System.Threading;

namespace VCInventoryRDP.NET
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
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

        private const string CHANNELNAME_SERVERCOMMAND = "SRVCMD";

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
            UInt32 TimeOut,
            [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] byte[] Buffer,
            UInt32 BufferSize, out UInt32 pBytesRead);

        [DllImport("wtsapi32.dll")]
        private static extern bool WTSVirtualChannelWrite(IntPtr hChannelHandle,
            byte[] Buffer, UInt32 Length, out UInt32 pBytesWritten);

        [DllImport("wtsapi32.dll")]
        private static extern int WTSVirtualChannelQuery(IntPtr hChannelHandle,
            int WtsVirtualClass, out IntPtr ppBuffer, out uint pBytesReturned);

        #endregion

        private IntPtr hChannelCommandHandle = IntPtr.Zero;

        protected override void OnStartup(StartupEventArgs e)
        {            
            base.OnStartup(e);

            string inputFile = null;
            string outputFile = null;

            string[] args = e.Args;

            foreach (string arg in args)
            {
                string[] argArray = arg.Split('|');
                if (argArray.Length > 1)
                {
                    string command = argArray[0];
                    string value = argArray[1];
                    if (command.Equals("/INFILE"))
                        inputFile = value;
                    else if (command.Equals("/OUTFILE"))
                        outputFile = value;

                }
            }

            string outString = ParseDatabaseXml(inputFile);

            if (outputFile != null && outString != null)
            {
                using (FileStream fs = new FileStream(outputFile, FileMode.Open))
                {
                    byte[] outBytes = Encoding.Unicode.GetBytes(outString);
                    fs.Write(outBytes, 0, outBytes.Length);
                }
            }

            this.Shutdown();
        }

        private string ParseDatabaseXml(string inFile)
        {
            try
            {
                if (string.IsNullOrWhiteSpace(inFile)) return null;

                //string connectionString = @"Provider=VFPOLEDB;Data Source='C:\Program Files (x86)\SupplyProDemo\dbf\ALLFIX.DBF';User ID=DEMO";
                string connectionString = @"Dsn=surgSupplyDb;uid=DEMO;pid=DEMO";

                XDocument inDocument = XDocument.Load(inFile);

                foreach (XElement node in inDocument.Root.XPathSelectElements("QUERY"))
                {
                    XElement sqlSelectElement = null;
                    XElement sqlDMLElement = null;

                    sqlSelectElement = node.XPathSelectElement("SELECT");
                    sqlDMLElement = node.XPathSelectElement("INSERT|DELETE|UPDATE");

                    string dataSetString = null;

                    using (OdbcConnection connection = new OdbcConnection(connectionString))
                    {
                        connection.Open();
                        using (OdbcCommand cmd = connection.CreateCommand())
                        {
                            try
                            {
                                if (sqlSelectElement != null && !string.IsNullOrWhiteSpace(sqlSelectElement.Value))
                                {
                                    DataSet set = new DataSet();
                                    cmd.CommandText = sqlSelectElement.Value.Trim();
                                    using (OdbcDataReader reader = cmd.ExecuteReader())
                                        set.Fill(reader);
                                    dataSetString = set.GetXml();
                                }
                                else if (sqlDMLElement != null && !string.IsNullOrWhiteSpace(sqlDMLElement.Value))
                                {
                                    cmd.CommandText = sqlDMLElement.Value.Trim();
                                    node.XPathSelectElement("ROWCOUNT").Value = cmd.ExecuteNonQuery().ToString();
                                }
                            }
                            catch (Exception ex)
                            {
                                node.XPathSelectElement("ERROR").Value = ex.Message;
                            }
                        }
                    }

                    if (dataSetString != null)
                    {
                        XmlReader xmlReader = XmlReader.Create(new StringReader(dataSetString));

                        while (xmlReader.Read())
                            node.LastNode.AddAfterSelf(XNode.ReadFrom(xmlReader));
                    }
                }

                return inDocument.ToString(SaveOptions.DisableFormatting);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                return null;
            }
        }
    }

    public static class ODBCExtensions
    {
        public static void Fill(this DataSet set, DbDataReader reader)
        {
            set.Tables.Add(new DataTable());

            for (int i = 0; i < reader.FieldCount; i++)
                set.Tables[0].Columns.Add(new DataColumn(reader.GetName(i), reader.GetFieldType(i)));

            while (reader.Read())
            {
                object[] data = new object[reader.FieldCount];

                for (int i = 0; i < reader.FieldCount; i++)
                    reader.GetValues(data);

                set.Tables[0].Rows.Add(data);
            }
        }
    }
}
