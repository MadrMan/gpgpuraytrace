using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace variableclient
{
    /// <summary>
    /// Interaction logic for ConnectBox.xaml
    /// </summary>
    public partial class ConnectBox : Window
    {
        public event Action<string> OnSetAddress;

        public ConnectBox()
        {
            InitializeComponent();
        }

        private void btnConnect_Click(object sender, RoutedEventArgs e)
        {
            e.Handled = true;
            OnSetAddress(tbAddress.Text);
        }

        bool isConnecting;
        public bool IsConnecting
        {
            get
            {
                return isConnecting;
            }
            set
            {
                isConnecting = value;

                if (isConnecting)
                {
                    btnConnect.IsEnabled = false;
                    btnConnect.Content = "...";
                }
                else
                {
                    btnConnect.IsEnabled = true;
                    btnConnect.Content = "Connect";
                }
            }
        }
    }
}
