namespace TestJig
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pbTestClientInfo = new System.Windows.Forms.Button();
            this.pbPingSrvCmd = new System.Windows.Forms.Button();
            this.pbPmInit = new System.Windows.Forms.Button();
            this.pbPmRelease = new System.Windows.Forms.Button();
            this.pbPmAcquire = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.pbTestDVC1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // pbTestClientInfo
            // 
            this.pbTestClientInfo.Location = new System.Drawing.Point(12, 12);
            this.pbTestClientInfo.Name = "pbTestClientInfo";
            this.pbTestClientInfo.Size = new System.Drawing.Size(95, 23);
            this.pbTestClientInfo.TabIndex = 0;
            this.pbTestClientInfo.Text = "Test Client Info";
            this.pbTestClientInfo.UseVisualStyleBackColor = true;
            this.pbTestClientInfo.Click += new System.EventHandler(this.pbTestClientInfo_Click);
            // 
            // pbPingSrvCmd
            // 
            this.pbPingSrvCmd.Location = new System.Drawing.Point(12, 41);
            this.pbPingSrvCmd.Name = "pbPingSrvCmd";
            this.pbPingSrvCmd.Size = new System.Drawing.Size(95, 23);
            this.pbPingSrvCmd.TabIndex = 1;
            this.pbPingSrvCmd.Text = "PingSrvCmd";
            this.pbPingSrvCmd.UseVisualStyleBackColor = true;
            this.pbPingSrvCmd.Click += new System.EventHandler(this.pbPingSrvCmd_Click);
            // 
            // pbPmInit
            // 
            this.pbPmInit.Location = new System.Drawing.Point(12, 87);
            this.pbPmInit.Name = "pbPmInit";
            this.pbPmInit.Size = new System.Drawing.Size(95, 23);
            this.pbPmInit.TabIndex = 2;
            this.pbPmInit.Text = "PM_INIT";
            this.pbPmInit.UseVisualStyleBackColor = true;
            this.pbPmInit.Click += new System.EventHandler(this.pbPmInit_Click);
            // 
            // pbPmRelease
            // 
            this.pbPmRelease.Location = new System.Drawing.Point(12, 116);
            this.pbPmRelease.Name = "pbPmRelease";
            this.pbPmRelease.Size = new System.Drawing.Size(95, 23);
            this.pbPmRelease.TabIndex = 3;
            this.pbPmRelease.Text = "PM_RELEASE";
            this.pbPmRelease.UseVisualStyleBackColor = true;
            this.pbPmRelease.Click += new System.EventHandler(this.pbPmRelease_Click);
            // 
            // pbPmAcquire
            // 
            this.pbPmAcquire.Location = new System.Drawing.Point(12, 145);
            this.pbPmAcquire.Name = "pbPmAcquire";
            this.pbPmAcquire.Size = new System.Drawing.Size(95, 23);
            this.pbPmAcquire.TabIndex = 4;
            this.pbPmAcquire.Text = "PM_ACQUIRE";
            this.pbPmAcquire.UseVisualStyleBackColor = true;
            this.pbPmAcquire.Click += new System.EventHandler(this.pbPmAcquire_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 186);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(95, 24);
            this.button1.TabIndex = 5;
            this.button1.Text = "Test Inventory";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // pbTestDVC1
            // 
            this.pbTestDVC1.Location = new System.Drawing.Point(177, 12);
            this.pbTestDVC1.Name = "pbTestDVC1";
            this.pbTestDVC1.Size = new System.Drawing.Size(95, 23);
            this.pbTestDVC1.TabIndex = 6;
            this.pbTestDVC1.Text = "Test DVC 1";
            this.pbTestDVC1.UseVisualStyleBackColor = true;
            this.pbTestDVC1.Click += new System.EventHandler(this.pbTestDVC1_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.pbTestDVC1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.pbPmAcquire);
            this.Controls.Add(this.pbPmRelease);
            this.Controls.Add(this.pbPmInit);
            this.Controls.Add(this.pbPingSrvCmd);
            this.Controls.Add(this.pbTestClientInfo);
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Test Jig x64";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button pbTestClientInfo;
        private System.Windows.Forms.Button pbPingSrvCmd;
        private System.Windows.Forms.Button pbPmInit;
        private System.Windows.Forms.Button pbPmRelease;
        private System.Windows.Forms.Button pbPmAcquire;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button pbTestDVC1;
    }
}

