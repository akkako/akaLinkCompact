import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog
import pywinusb.hid as hid
import threading
import time
import json
from datetime import datetime


class SendRow:
    def __init__(self, parent, row_index, delete_callback):
        self.row_index = row_index
        self.delete_callback = delete_callback
        
        self.frame = ttk.Frame(parent)
        
        self.index_label = ttk.Label(self.frame, text=f"{row_index + 1}.", width=3)
        self.index_label.pack(side=tk.LEFT, padx=2)
        
        self.mode_var = tk.StringVar(value="ASCII")
        self.mode_combo = ttk.Combobox(self.frame, textvariable=self.mode_var, 
                                        values=["ASCII", "HEX"], state="readonly", width=8)
        self.mode_combo.pack(side=tk.LEFT, padx=2)
        
        self.entry = ttk.Entry(self.frame, width=50)
        self.entry.pack(side=tk.LEFT, padx=2, fill=tk.X, expand=True)
        
        self.send_btn = ttk.Button(self.frame, text="发送", width=8)
        self.send_btn.pack(side=tk.LEFT, padx=2)
        
        self.delete_btn = ttk.Button(self.frame, text="X", width=3, command=self.on_delete)
        self.delete_btn.pack(side=tk.LEFT, padx=2)
    
    def on_delete(self):
        self.delete_callback(self)
    
    def get_data(self):
        content = self.entry.get().strip()
        mode = self.mode_var.get()
        if not content:
            return None
        
        try:
            if mode == "ASCII":
                data = list(content.encode('utf-8'))
            else:
                hex_str = content.replace(' ', '').replace(',', '')
                data = [int(hex_str[i:i+2], 16) for i in range(0, len(hex_str), 2) if len(hex_str[i:i+2]) == 2]
            return data
        except Exception:
            return None
    
    def get_config(self):
        return {
            'mode': self.mode_var.get(),
            'data': self.entry.get()
        }
    
    def set_config(self, config):
        self.mode_var.set(config.get('mode', 'ASCII'))
        self.entry.delete(0, tk.END)
        self.entry.insert(0, config.get('data', ''))


class HIDToolApp:
    def __init__(self, root):
        self.root = root
        self.root.title("USB HID 通信工具")
        self.root.geometry("1000x850")
        
        self.device = None
        self.connected = False
        self.receive_thread = None
        self.running = False
        self.send_rows = []
        self.report_id = 1
        self.report_size = 64
        self.device_info = {}
        
        self.setup_ui()
        self.refresh_devices()
        self.add_send_row()
    
    def truncate_str(self, s, max_len):
        if not s:
            return "Unknown"
        if len(s) > max_len:
            return s[:max_len-2] + ".."
        return s
    
    def setup_ui(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        
        device_frame = ttk.LabelFrame(main_frame, text="设备连接", padding="10")
        device_frame.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)
        
        ttk.Label(device_frame, text="HID设备:").grid(row=0, column=0, sticky=tk.W, padx=5)
        
        self.device_combo = ttk.Combobox(device_frame, width=75, state="readonly", font=("Consolas", 9))
        self.device_combo.grid(row=0, column=1, sticky=(tk.W, tk.E), padx=5)
        
        self.refresh_btn = ttk.Button(device_frame, text="刷新", command=self.refresh_devices, width=10)
        self.refresh_btn.grid(row=0, column=2, padx=5)
        
        self.connect_btn = ttk.Button(device_frame, text="连接", command=self.toggle_connection, width=10)
        self.connect_btn.grid(row=0, column=3, padx=5)
        
        self.status_label = ttk.Label(device_frame, text="未连接", foreground="red")
        self.status_label.grid(row=0, column=4, padx=10)
        
        device_frame.columnconfigure(1, weight=1)
        
        self.paned = ttk.PanedWindow(main_frame, orient=tk.VERTICAL)
        self.paned.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        send_frame = ttk.LabelFrame(self.paned, text="发送区", padding="10")
        self.paned.add(send_frame)
        
        self.send_canvas = tk.Canvas(send_frame)
        self.send_scrollbar = ttk.Scrollbar(send_frame, orient="vertical", command=self.send_canvas.yview)
        self.send_scrollable_frame = ttk.Frame(self.send_canvas)
        
        self.send_scrollable_frame.bind(
            "<Configure>",
            lambda e: self.send_canvas.configure(scrollregion=self.send_canvas.bbox("all"))
        )
        
        self.send_canvas.create_window((0, 0), window=self.send_scrollable_frame, anchor="nw")
        self.send_canvas.configure(yscrollcommand=self.send_scrollbar.set)
        
        self.send_canvas.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.send_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        
        send_btn_frame = ttk.Frame(send_frame)
        send_btn_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        self.add_send_btn = ttk.Button(send_btn_frame, text="+ 添加", command=self.add_send_row, width=10)
        self.add_send_btn.pack(side=tk.LEFT, padx=5)
        
        self.import_btn = ttk.Button(send_btn_frame, text="导入", command=self.import_send_data, width=10)
        self.import_btn.pack(side=tk.LEFT, padx=5)
        
        self.export_btn = ttk.Button(send_btn_frame, text="导出", command=self.export_send_data, width=10)
        self.export_btn.pack(side=tk.LEFT, padx=5)
        
        self.add_length_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(send_btn_frame, text="发+长度(第2字节)", variable=self.add_length_var).pack(side=tk.LEFT, padx=20)
        
        send_frame.columnconfigure(0, weight=1)
        send_frame.rowconfigure(0, weight=1)
        
        receive_frame = ttk.LabelFrame(self.paned, text="接收区", padding="10")
        self.paned.add(receive_frame)
        
        receive_config_frame = ttk.Frame(receive_frame)
        receive_config_frame.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Label(receive_config_frame, text="显示:").pack(side=tk.LEFT, padx=5)
        
        self.receive_mode_var = tk.StringVar(value="BOTH")
        ttk.Radiobutton(receive_config_frame, text="ASCII+HEX", variable=self.receive_mode_var, 
                       value="BOTH", command=self.on_receive_mode_change).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(receive_config_frame, text="仅ASCII", variable=self.receive_mode_var, 
                       value="ASCII", command=self.on_receive_mode_change).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(receive_config_frame, text="仅HEX", variable=self.receive_mode_var, 
                       value="HEX", command=self.on_receive_mode_change).pack(side=tk.LEFT, padx=5)
        
        ttk.Separator(receive_config_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        self.parse_length_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(receive_config_frame, text="收-长度(第2字节)", variable=self.parse_length_var).pack(side=tk.LEFT, padx=5)
        
        self.hide_sent_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(receive_config_frame, text="隐藏发送数据", variable=self.hide_sent_var).pack(side=tk.LEFT, padx=5)
        
        ttk.Separator(receive_config_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        self.show_time_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(receive_config_frame, text="显示时间", variable=self.show_time_var).pack(side=tk.LEFT, padx=5)
        
        ttk.Separator(receive_config_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        ttk.Label(receive_config_frame, text="包长度:").pack(side=tk.LEFT, padx=5)
        self.packet_size_label = ttk.Label(receive_config_frame, text="64", foreground="blue")
        self.packet_size_label.pack(side=tk.LEFT, padx=5)
        
        self.receive_paned = ttk.PanedWindow(receive_frame, orient=tk.HORIZONTAL)
        self.receive_paned.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        ascii_frame = ttk.Frame(self.receive_paned)
        self.receive_paned.add(ascii_frame, weight=1)
        
        self.receive_ascii_text = scrolledtext.ScrolledText(ascii_frame, height=10, font=("Consolas", 10))
        self.receive_ascii_text.pack(fill=tk.BOTH, expand=True)
        
        hex_frame = ttk.Frame(self.receive_paned)
        self.receive_paned.add(hex_frame, weight=1)
        
        self.receive_hex_text = scrolledtext.ScrolledText(hex_frame, height=10, font=("Consolas", 10))
        self.receive_hex_text.pack(fill=tk.BOTH, expand=True)
        
        receive_btn_frame = ttk.Frame(receive_frame)
        receive_btn_frame.grid(row=2, column=0, sticky=(tk.W, tk.E), pady=5)
        
        self.clear_receive_btn = ttk.Button(receive_btn_frame, text="清空接收区", command=self.clear_receive, width=15)
        self.clear_receive_btn.pack(side=tk.LEFT, padx=5)
        
        self.auto_scroll_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(receive_btn_frame, text="自动滚动", variable=self.auto_scroll_var).pack(side=tk.LEFT, padx=20)
        
        receive_frame.columnconfigure(0, weight=1)
        receive_frame.rowconfigure(1, weight=1)
        
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(1, weight=1)
        
        self.root.after(100, self.on_receive_mode_change)
    
    def on_receive_mode_change(self):
        mode = self.receive_mode_var.get()
        self.receive_paned.grid()
        
        self.root.update_idletasks()
        width = self.receive_paned.winfo_width()
        if width < 100:
            width = 800
        
        if mode == "BOTH":
            self.receive_paned.sashpos(0, width // 2)
        elif mode == "ASCII":
            self.receive_paned.sashpos(0, width - 10)
        else:
            self.receive_paned.sashpos(0, 10)
    
    def add_send_row(self):
        row = SendRow(self.send_scrollable_frame, len(self.send_rows), self.remove_send_row)
        row.frame.pack(fill=tk.X, pady=2)
        row.send_btn.config(command=lambda: self.send_single_row(row))
        self.send_rows.append(row)
    
    def remove_send_row(self, row):
        if len(self.send_rows) <= 1:
            messagebox.showwarning("警告", "至少保留一个发送框")
            return
        
        row.frame.destroy()
        self.send_rows.remove(row)
        
        for i, r in enumerate(self.send_rows):
            r.index_label.config(text=f"{i + 1}.")
            r.row_index = i
    
    def export_send_data(self):
        if not self.send_rows:
            return
        
        data = []
        for row in self.send_rows:
            data.append(row.get_config())
        
        filepath = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON文件", "*.json"), ("所有文件", "*.*")]
        )
        
        if filepath:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    json.dump(data, f, indent=2, ensure_ascii=False)
                messagebox.showinfo("导出成功", f"已导出到: {filepath}")
            except Exception as e:
                messagebox.showerror("导出失败", str(e))
    
    def import_send_data(self):
        filepath = filedialog.askopenfilename(
            filetypes=[("JSON文件", "*.json"), ("所有文件", "*.*")]
        )
        
        if filepath:
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                
                for row in self.send_rows:
                    row.frame.destroy()
                self.send_rows.clear()
                
                for config in data:
                    row = SendRow(self.send_scrollable_frame, len(self.send_rows), self.remove_send_row)
                    row.frame.pack(fill=tk.X, pady=2)
                    row.send_btn.config(command=lambda r=row: self.send_single_row(r))
                    row.set_config(config)
                    self.send_rows.append(row)
                
                if not self.send_rows:
                    self.add_send_row()
                    
                messagebox.showinfo("导入成功", f"已从: {filepath} 导入")
            except Exception as e:
                messagebox.showerror("导入失败", str(e))
    
    def send_single_row(self, row):
        if not self.connected or not self.device:
            messagebox.showwarning("警告", "请先连接设备")
            return
        
        data = row.get_data()
        if data is None:
            messagebox.showwarning("警告", "请输入有效数据")
            return
        
        try:
            packet = [self.report_id]
            
            if self.add_length_var.get():
                data_len = len(data)
                if data_len > 255:
                    data_len = 255
                packet.append(data_len)
            
            packet.extend(data)
            
            if len(packet) < self.report_size:
                packet.extend([0] * (self.report_size - len(packet)))
            elif len(packet) > self.report_size:
                packet = packet[:self.report_size]
            
            self.device.send_output_report(packet)
            
            display_data = row.entry.get()
            if self.add_length_var.get():
                display_data += f" (长度:{len(data)})"
            
            if not self.hide_sent_var.get():
                msg = f"[发送成功] {display_data}\n"
                self.append_receive_text(msg, msg)
        except Exception as e:
            messagebox.showerror("发送错误", str(e))
    
    def append_receive_text(self, ascii_msg, hex_msg):
        mode = self.receive_mode_var.get()
        
        self.receive_ascii_text.insert(tk.END, ascii_msg)
        self.receive_hex_text.insert(tk.END, hex_msg)
        
        if self.auto_scroll_var.get():
            self.receive_ascii_text.see(tk.END)
            self.receive_hex_text.see(tk.END)
    
    def refresh_devices(self):
        try:
            all_devices = hid.HidDeviceFilter().get_devices()
            device_list = []
            self.device_dict = {}
            self.device_info_dict = {}
            
            for dev in all_devices:
                vendor_id = dev.vendor_id
                product_id = dev.product_id
                
                try:
                    dev.open()
                    manufacturer = dev.vendor_name if hasattr(dev, 'vendor_name') and dev.vendor_name else ''
                    product_name = dev.product_name if hasattr(dev, 'product_name') and dev.product_name else ''
                    serial = dev.serial_number if hasattr(dev, 'serial_number') and dev.serial_number else ''
                    dev.close()
                except:
                    manufacturer = ''
                    product_name = ''
                    serial = ''
                
                manufacturer = self.truncate_str(manufacturer, 12)
                product_name = self.truncate_str(product_name, 20)
                serial = self.truncate_str(serial, 25) if serial else 'N/A'
                
                desc = f"VID:{vendor_id:04X} PID:{product_id:04X} | {manufacturer:12} | {product_name:20} | SN:{serial:25}"
                
                device_list.append(desc)
                self.device_dict[desc] = dev
                self.device_info_dict[desc] = {
                    'vid': vendor_id,
                    'pid': product_id,
                    'manufacturer': manufacturer,
                    'product': product_name,
                    'serial': serial
                }
            
            self.device_combo['values'] = device_list
            
            if device_list:
                self.device_combo.current(0)
                self.status_label.config(text=f"找到 {len(device_list)} 个设备", foreground="orange")
            else:
                self.status_label.config(text="未找到设备", foreground="red")
                
        except Exception as e:
            messagebox.showerror("错误", f"枚举设备失败: {str(e)}")
    
    def toggle_connection(self):
        if self.connected:
            self.disconnect()
        else:
            self.connect()
    
    def connect(self):
        selected = self.device_combo.get()
        if not selected:
            messagebox.showwarning("警告", "请选择一个HID设备")
            return
        
        try:
            dev = self.device_dict[selected]
            dev.open()
            
            output_reports = dev.find_output_reports()
            if output_reports:
                self.report_id = output_reports[0].report_id
            
            caps = dev.hid_caps
            self.report_size = max(
                caps.output_report_byte_length if caps.output_report_byte_length > 0 else 64,
                caps.input_report_byte_length if caps.input_report_byte_length > 0 else 64
            )
            
            self.packet_size_label.config(text=str(self.report_size))
            
            self.device_info = self.device_info_dict.get(selected, {})
            
            def rx(data):
                self.root.after(0, self.display_received_data, list(data))
            
            dev.set_raw_data_handler(rx)
            
            self.device = dev
            self.connected = True
            self.running = True
            
            self.connect_btn.config(text="断开")
            self.status_label.config(text="已连接", foreground="green")
            self.device_combo.config(state="disabled")
            self.refresh_btn.config(state="disabled")
            
        except Exception as e:
            messagebox.showerror("连接错误", f"无法连接设备: {str(e)}")
            if self.device:
                try:
                    self.device.close()
                except:
                    pass
            self.device = None
    
    def disconnect(self):
        self.running = False
        self.connected = False
        
        if self.device:
            try:
                self.device.close()
            except:
                pass
        self.device = None
        
        self.connect_btn.config(text="连接")
        self.status_label.config(text="未连接", foreground="red")
        self.device_combo.config(state="readonly")
        self.refresh_btn.config(state="normal")
        self.packet_size_label.config(text="64")
        
        time.sleep(0.2)
    
    def display_received_data(self, data):
        if not data or len(data) < 1:
            return
        
        mode = self.receive_mode_var.get()
        show_time = self.show_time_var.get()
        
        time_str = ""
        if show_time:
            time_str = f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] "
        
        report_id = data[0]
        payload = data[1:] if len(data) > 1 else []
        
        if self.parse_length_var.get() and len(payload) >= 1:
            length_byte = payload[0]
            if 0 < length_byte < len(payload):
                payload = payload[1:1+length_byte]
        
        hex_str = ' '.join(f'{b:02X}' for b in payload)
        
        try:
            text = ''.join(chr(b) if 32 <= b < 127 else '.' for b in payload)
            ascii_msg = f"{time_str}ID:{report_id:02X} ASCII: {text}\n"
            hex_msg = f"{time_str}ID:{report_id:02X} HEX: {hex_str}\n"
        except:
            ascii_msg = f"{time_str}ID:{report_id:02X} HEX: {hex_str}\n"
            hex_msg = ascii_msg
        
        mode = self.receive_mode_var.get()
        
        if mode == "BOTH":
            self.append_receive_text(ascii_msg, hex_msg)
        elif mode == "ASCII":
            self.append_receive_text(ascii_msg, hex_msg)
        else:
            self.append_receive_text(ascii_msg, hex_msg)
    
    def display_error(self, error_msg):
        msg = f"[错误] {error_msg}\n"
        self.append_receive_text(msg, msg)
    
    def clear_receive(self):
        self.receive_ascii_text.delete("1.0", tk.END)
        self.receive_hex_text.delete("1.0", tk.END)
    
    def on_closing(self):
        self.running = False
        if self.device:
            try:
                self.device.close()
            except:
                pass
        self.root.destroy()


def main():
    root = tk.Tk()
    app = HIDToolApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
