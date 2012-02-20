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
        bool closing = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            varman = new VariableManager();
            widgets = new Dictionary<Variable, Widget>();

            this.Closing += new System.ComponentModel.CancelEventHandler(MainWindow_Closing);

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

        void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            closing = true;

            varman.Stop();
        }

        void connectBox_Closed(object sender, EventArgs e)
        {
            if (!closing)
            {
                this.Close();
            }
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
            Dispatcher.Invoke(new Action(() =>
            {
                if (connectBox.Visibility == Visibility.Visible)
                {
                    connectBox.Hide();
                }
            }), null);
        }

        void connectBox_OnSetAddress(string address)
        {
            connectBox.IsConnecting = true;
            varman.Connect(address);
        }

        void varman_OnConnectionLost()
        {
            if (!closing)
            {
                Dispatcher.Invoke(new Action(() =>
                {
                    connectBox.IsConnecting = false;
                    connectBox.Show();
                }), null);
            }
        }

        void widget_OnChangeVariable(Variable v)
        {
            varman.SendVariable(v);
        }

        void VariableAdded(Variable variable)
        {
            Dispatcher.Invoke(new Action<MainWindow>((sender) =>
            {
                Widget widget = new Widget(variable);
                widget.OnChangeVariable += new Action<Variable>(widget_OnChangeVariable);

                wrapPanel1.Children.Add(widget);
                widgets[variable] = widget;
            }), this); 
        }

        void VariableRemoved(Variable variable)
        {
            Dispatcher.Invoke(new Action<MainWindow>((sender) =>
            {
                Widget widget = widgets[variable];

                wrapPanel1.Children.Remove(widget);
                widgets.Remove(variable);
            }), this);
        }
    }
}
