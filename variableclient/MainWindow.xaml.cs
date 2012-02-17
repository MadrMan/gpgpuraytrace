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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace variableclient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        VariableManager varman;
        Dictionary<Variable, Widget> widgets;
        ConnectBox connectBox = new ConnectBox();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            varman = new VariableManager();
            widgets = new Dictionary<Variable, Widget>();

            varman.OnVariableAdded += new VariableManager.DlgVariableEvent(VariableAdded);
            varman.OnVariableRemoved += new VariableManager.DlgVariableEvent(VariableRemoved);
            varman.OnConnectionLost += new Action(varman_OnConnectionLost);
            varman.OnConnected += new Action(varman_OnConnected);
            varman.OnConnectionFailed += new Action(varman_OnConnectionFailed);

            connectBox.OnSetAddress += new Action<string>(connectBox_OnSetAddress);
            connectBox.Closed += new EventHandler(connectBox_Closed);
            connectBox.Owner = this;

            varman.Start();
        }

        void connectBox_Closed(object sender, EventArgs e)
        {
            this.Close();
        }

        void varman_OnConnectionFailed()
        {
            Dispatcher.Invoke(new Action(() =>
            {
                connectBox.IsConnecting = false;
            }), null);
        }

        void varman_OnConnected()
        {
            if (connectBox.Visibility == Visibility.Visible)
            {
                connectBox.Hide();
            }
        }

        void connectBox_OnSetAddress(string address)
        {
            connectBox.IsConnecting = true;
            varman.Connect(address);
        }

        void varman_OnConnectionLost()
        {
            Dispatcher.Invoke(new Action(() =>
            {
                connectBox.IsConnecting = false;
                connectBox.Owner = this;
                connectBox.Show();
            }), null);
        }

        Widget CreateWidget(Variable variable)
        {
            Widget widget = null;
            Dispatcher.Invoke(new Action<MainWindow>((sender) =>
            {
                widget = new Widget(variable);
                wrapPanel1.Children.Add(widget);
                widgets[variable] = widget;
            }), this);
            return widget;
        }

        void VariableAdded(Variable variable)
        {
            Widget widget = CreateWidget(variable);
        }

        void VariableRemoved(Variable variable)
        {
            Widget widget = widgets[variable];
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            varman.Stop();
        }
    }
}
