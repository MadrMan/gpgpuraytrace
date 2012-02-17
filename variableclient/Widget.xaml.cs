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
using System.Reflection;

namespace variableclient
{
    /// <summary>
    /// Interaction logic for Widget.xaml
    /// </summary>
    public partial class Widget : UserControl
    {
        Variable variable;

        Brush colorCurrent = Brushes.LightGreen;
        Brush colorModified = Brushes.White;
        Brush colorIncorrect = Brushes.PaleVioletRed;

        class WidgetVariable
        {
            public Variable variable;
            public FieldInfo field;
        }

        public Widget(Variable variable)
        {
            this.variable = variable;

            InitializeComponent();
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            lblName.Content = variable.name;
            FieldInfo[] fields = variable.data.GetType().GetFields();

            int rowIndex = 0;
            foreach (FieldInfo m in fields)
            {
                RowDefinition row = new RowDefinition();
                row.Height = new GridLength(0, GridUnitType.Auto);
                grid.RowDefinitions.Add(row);
                rowIndex++;

                Label varLabel = new Label();
                varLabel.Content = m.Name;
                varLabel.HorizontalContentAlignment = HorizontalAlignment.Right;
                varLabel.VerticalContentAlignment = VerticalAlignment.Center;
                varLabel.Margin = new Thickness(0, 0, 5, 0);
                varLabel.Height = 23;

                WidgetVariable wv = new WidgetVariable();
                wv.variable = variable;
                wv.field = m;

                UIElement control = null;

                if (m.FieldType == typeof(float))
                {
                    TextBox tb = new TextBox();
                    tb.Height = varLabel.Height;
                    tb.KeyDown += new KeyEventHandler(tb_KeyDown);
                    tb.TextChanged += new TextChangedEventHandler(tb_TextChanged);
                    tb.Tag = wv;
                    tb.Background = colorCurrent;

                    control = tb;
                }

                grid.Children.Add(control);
                Grid.SetColumn(control, 1);
                Grid.SetRow(control, rowIndex);

                Grid.SetColumn(varLabel, 0);
                Grid.SetRow(varLabel, rowIndex);
                grid.Children.Add(varLabel);
            }
        }

        void tb_TextChanged(object sender, TextChangedEventArgs e)
        {
            TextBox tb = sender as TextBox;
            tb.Background = colorModified;
        }

        void tb_KeyDown(object sender, KeyEventArgs e)
        {
            TextBox tb = sender as TextBox;
            WidgetVariable wv = tb.Tag as WidgetVariable;

            //tb.Background = colorModified;

            if (e.Key == Key.Enter)
            {
                e.Handled = true;

                float value;
                if(!float.TryParse(tb.Text, out value))
                {
                    MessageBox.Show("Invalid value for " + wv.variable.name + "." + wv.field.Name);
                    tb.Background = colorIncorrect;

                    return;
                }

                tb.Background = colorCurrent;
                wv.field.SetValue(wv.variable.data, value);
            }
        }
    }
}
