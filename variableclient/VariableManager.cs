using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Windows;

namespace variableclient
{
    public class Variable
    {
        public String name;
        public String type;
        public Object data;
    }

    public class VariableManager
    {
        private const int port = 10666;

        public delegate void DlgVariableEvent(Variable variable);
        public event DlgVariableEvent OnVariableAdded;
        public event DlgVariableEvent OnVariableRemoved;
        public event Action OnConnectionLost;
        public event Action OnConnected;
        public event Action OnConnectionFailed;

        private TcpClient client;
        private BinaryReader clientReader;
        private AutoResetEvent eventConnected = new AutoResetEvent(false);

        List<Variable> variables;
        List<Variable> Variables
        {
            get { return variables; }
        }

        public String Address
        {
            get;
            private set;
        }

        unsafe void dataToVariable(Variable v, byte[] data)
        {
            Type[] types = typeof(Native).GetNestedTypes();
            Type native = types.Where(t => t.Name == "_" + v.type).SingleOrDefault();

            if (native == null)
            {
                throw new Exception("Unsupported type send from server");
            }

            unsafe
            {
                fixed (byte* d = data)
                {
                    v.data = Marshal.PtrToStructure((IntPtr)d, native);
                }
            }
        }

        public VariableManager()
        {
            variables = new List<Variable>();
            client = new TcpClient();
        }

        bool stopping;
        public void Start()
        {
            stopping = false;
            new Thread(Run).Start();
        }

        public void Stop()
        {
            stopping = true;
            eventConnected.Set();
        }

        private void AddVariable(string name, string type, byte[] data)
        {
            Variable v = new Variable();
            v.name = name;
            v.type = type;
            dataToVariable(v, data);
            variables.Add(v);
            OnVariableAdded(v);
        }

        private void RemoveVariable(string name)
        {
            Variable variable = variables.Find((v) => v.name == name);
            variables.Remove(variable);
            OnVariableRemoved(variable);
        }

        /// <summary>
        /// Run the variable manager which handles the networking
        /// 
        /// -Packet format-
        /// TO server:
        /// [var:s][data:x]
        /// FROM server:
        /// [add(1)][var:s][type:s][length:2][data:x]
        /// [remove(0)][var:s]
        /// </summary>
        private void Run()
        {
            /*Variable v = new Variable();
            v.name = "myvariable";
            v.type = "float";
            byte[] data = BitConverter.GetBytes(20.0f);
            dataToVariable(v, data);
            variables.Add(v);
            OnVariableAdded(v);

            v = new Variable();
            v.name = "myvariable2";
            v.type = "float3";
            data = data.Concat(BitConverter.GetBytes(40.0f)).ToArray();
            data = data.Concat(BitConverter.GetBytes(70.0f)).ToArray();
            dataToVariable(v, data);
            variables.Add(v);
            OnVariableAdded(v);*/

            OnConnectionLost();

            while (!stopping)
            {
                if (!client.Connected)
                {
                    eventConnected.WaitOne();
                }
                else
                {
                    int packet = clientReader.ReadByte();
                    int length;
                    string variable, type;
                    byte[] data;

                    switch (packet)
                    {
                        case 0:
                            break;
                        case 1:
                            length = clientReader.ReadByte();
                            variable = System.Text.ASCIIEncoding.ASCII.GetString(clientReader.ReadBytes(length));
                            length = clientReader.ReadByte();
                            type = System.Text.ASCIIEncoding.ASCII.GetString(clientReader.ReadBytes(length));
                            int dataLength = clientReader.ReadInt16();
                            data = clientReader.ReadBytes(dataLength);

                            AddVariable(variable, type, data);
                            break;
                        default:
                            length = clientReader.ReadByte();
                            variable = System.Text.ASCIIEncoding.ASCII.GetString(clientReader.ReadBytes(length));
                            RemoveVariable(variable);
                            break;
                    }

                    if (!client.Connected) OnConnectionLost();
                }
            }
        }

        internal void Connect(string address)
        {
            Address = address;
            client.BeginConnect(address, port, (ar) =>
            {
                try
                {
                    client.EndConnect(ar);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }

                if (client.Connected)
                {
                    OnConnected();

                    clientReader = new BinaryReader(client.GetStream());
                    eventConnected.Set();
                }
                else
                {
                    OnConnectionFailed();
                }
            }, null);
        }
    }
}
