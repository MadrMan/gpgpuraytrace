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
    public unsafe class Variable
    {
        public Object data;
        private byte[] dataArray;
        private IntPtr dataPointer;
        private Type type;
        private String name;

        public byte[] DataArray
        {
            get { return dataArray; }
        }

        /*public Type Type
        {
            get { return type; }
        }*/

        public String Name
        {
            get { return name; }
        }

        public Variable(Type type, String name)
        {
            this.type = type;
            this.name = name;

            dataPointer = Marshal.AllocHGlobal(Marshal.SizeOf(type));
        }

        public void StructToData()
        {
            Marshal.StructureToPtr(data, dataPointer, false);
            Marshal.Copy(dataPointer, dataArray, 0, dataArray.Length);
        }

        public void DataToStruct(byte[] array)
        {
            dataArray = array;

            fixed (byte* d = dataArray)
            {
                data = Marshal.PtrToStructure((IntPtr)d, type);
            }
        }
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
        private BinaryWriter clientWriter;
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

        }

        public VariableManager()
        {
            variables = new List<Variable>();
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
            if (client != null) client.Close();
        }

        private void AddVariable(string name, string type, byte[] data)
        {
            Type[] types = typeof(Native).GetNestedTypes();
            Type native = types.Where(t => t.Name == "_" + type).SingleOrDefault();

            if (native == null)
            {
                throw new Exception("Unsupported type send from server");
            }

            Variable v = new Variable(native, name);
            v.DataToStruct(data);

            variables.Add(v);
            OnVariableAdded(v);
        }

        private void RemoveVariable(string name)
        {
            Variable variable = variables.Find((v) => v.Name == name);
            variables.Remove(variable);
            OnVariableRemoved(variable);
        }

        private void ClearVariables()
        {
            foreach (Variable v in variables)
                OnVariableRemoved(v);
            variables.Clear();
        }

        /// <summary>
        /// Run the variable manager which handles the networking
        /// 
        /// -Packet format-
        /// TO server:
        /// [var:s][data:x]
        /// FROM server:
        /// 
        /// [add(1):1][var:s][type:s][length:2][data:x]
        /// [remove(0):1][var:s]
        /// [removeAll(2):1][var:s]
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
                if (client == null)
                {
                    eventConnected.WaitOne();
                }
                else
                {
                    int packet = -1;
                    try
                    {
                        packet = clientReader.ReadByte();

                        int length;
                        string variable, type;
                        byte[] data;

                        switch (packet)
                        {
                            case 0:
                                length = clientReader.ReadByte();
                                variable = System.Text.ASCIIEncoding.ASCII.GetString(clientReader.ReadBytes(length));
                                RemoveVariable(variable);
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
                            case 2:
                                ClearVariables();
                                break;
                            default:
                                break;
                        }
                    }
                    catch (IOException)
                    {
                        client.Close();
                        client = null;
                    }

                    if (client == null)
                    {
                        ClearVariables();
                        OnConnectionLost();
                    }
                }
            }
        }

        internal void Connect(string address)
        {
            Address = address;
            
            if (address == "")
                address = "localhost";

            client = new TcpClient();
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
                    clientWriter = new BinaryWriter(client.GetStream());
                    eventConnected.Set();
                }
                else
                {
                    OnConnectionFailed();
                }
            }, null);
        }

        internal void SendVariable(Variable v)
        {
            v.StructToData();

            lock (clientWriter)
            {
                clientWriter.Write((byte)v.Name.Length);
                clientWriter.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(v.Name));
                clientWriter.Write(v.DataArray, 0, v.DataArray.Length);
                client.GetStream().Flush();
            }
        }
    }
}
