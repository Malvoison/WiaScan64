using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.Runtime.InteropServices;
using System.Diagnostics;
using WiaScanServer;

namespace TestJig
{
    public partial class Form1 : Form
    {
        #region Dll Imports

        //[DllImport("vcscanwiardp.dll", CharSet = CharSet.Unicode, EntryPoint = "WTSVirtualChannelOpenQ")]
        //private static extern IntPtr WTSVirtualChannelOpen(IntPtr hServer,
        //    UInt32 SessionId, string pVirtualName);

        //[DllImport("vcscanwiardp.dll", EntryPoint = "WTSVirtualChannelCloseQ")]
        //private static extern Int32 WTSVirtualChannelClose(IntPtr hChannelHandle);

        //[DllImport("vcscanwiardp.dll", EntryPoint = "WTSVirtualChannelReadQ")]
        //private static extern Int32 WTSVirtualChannelRead(IntPtr hChannelHandle,
        //    UInt32 TimeOut, byte[] Buffer, UInt32 BufferSize, out UInt32 pBytesRead);

        //[DllImport("vcscanwiardp.dll", EntryPoint = "WTSVirtualChannelWriteQ")]
        //private static extern Int32 WTSVirtualChannelWrite(IntPtr hChannelHandle,
        //    byte[] Buffer, UInt32 Length, out UInt32 pBytesWritten);

        #endregion

        #region #defines

        private const string CHANNEL_NAME = "VCScan";
        private const int SCAN_VER = 3;

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
        private const int PM_PING = WM_APP + 0x0100;
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

        #endregion

        #region Private Variables

        private WiaScanServer.WiaScanServer _wsc = null;

        #endregion

        public Form1()
        {
            InitializeComponent();
        }

        private void pbTestClientInfo_Click(object sender, EventArgs e)
        {
            try
            {
                _wsc = WiaScanServer.WiaScanServer.GetInstance();
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        private void pbPingSrvCmd_Click(object sender, EventArgs e)
        {
            try
            {
                _wsc = WiaScanServer.WiaScanServer.GetInstance();
                _wsc.SendServerCommand();
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        private void pbPmInit_Click(object sender, EventArgs e)
        {
            try
            {
                _wsc = WiaScanServer.WiaScanServer.GetInstance();
                _wsc.SendServerCommand(PM_INIT);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        private void pbPmRelease_Click(object sender, EventArgs e)
        {
            try
            {
                _wsc = WiaScanServer.WiaScanServer.GetInstance();
                _wsc.SendServerCommand(PM_RELEASE);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }

        private void pbPmAcquire_Click(object sender, EventArgs e)
        {
            try
            {
                _wsc = WiaScanServer.WiaScanServer.GetInstance();
                _wsc.SendServerCommand(PM_ACQUIRE);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                throw;
            }
        }


        //InventoryServer.InventoryServer server = InventoryServer.InventoryServer.GetInstance();

        private void button1_Click(object sender, EventArgs e)
        {
            //server.DataReceivedEvent += server_DataReceivedEvent;

            //server.ExecuteDataSet("Select * from allfix");
        }

        private void pbTestDVC1_Click(object sender, EventArgs e)
        {

        }

        //private void server_DataReceivedEvent(object sender, InventoryServer.InventoryServer.DataReceivedEventArgs args)
        //{
        //    server.DataReceivedEvent -= server_DataReceivedEvent;
        //}
    }
}
