This project is for Three-Phase Power Meter System using esp32 and pzem modules.
The hardware serial 2 will be used to connect to pzem vis IC hexa buffer, 74126.
Hardware Serial 2 uses Pin 16(Rx) and Pin 17(Tx) connect to 74126.
Pin5, 18, 19 are used for selecting pzem module.
ESP32 will send bluetooth signal for configuraing wifi SSID and Password, including
sending data IP address, with folder and subfolder
Url format:
ip.address/folder/subfolder/insert3E.php?devid=..&mcid=..&a1=..&v1=..&pf1=..&e1=..&a2=..&v2=..&pf2=..&e2=..&a3=..&v3=..&pf3=..&e3=..
How to cinnect to bluetooth:
- open setting in your mobile phone
- open bluetooth and find esp32 device (bluetooth name is inside the box)
- pairing the esp32 device
- install Serial Bluetooth Terminal (SBT) Application from App Store, or Play Store
- open SBT app click three equal sign on the top left
- click Devices and select Bluetooth LE tab
- select the paired device
- when connected there is info text roll up.
- key i and enter for information and command.
- information:
  mcAddress: xx:xx:xx:xx:xx:xx
 "A SSID" 
 "B Pass"
 "C Server"
 "D Path"
 "E Device"
 "F WiFi"
 "G t2server"
 "I Print this menu"
 "N BLE beacon name"
 "M Machine name"
 "R Reset settings"
 "Z Reset MCU"

- factory default:
 A SSID: TP-Link_5B9A
 B Pass: 97180937
 C Server IP: 137.184.86.182
 D Path: iot2025
 E Device: e003
 F Wifi(0,1): 1
 G t2server: 60
 I Print Info
 N BLE name: e32-p3-003
 M Machine ID: MDB-003
 r: Reset Ble
 z: Restart MCU

- command:(end with ';')
- Assidname;  //change ssid name e.g. ATP-Link_5B9A;  -->change SSID name to TP-Link_5B9A 
- Bpassword;  //change ssid password e.g. B97180937;  -->change ssid password to 97180937
- Cserveripaddress; //change server ip address e.g. C192.168.1.100;  --> change server address to 192.168.1.100
- Dfolder; //folder of the server file  e.g. Diot2025;  --> change folder to iot2025
- Esubfolder; //subfolder of the server file  e.g. Ee003;  --> change subfolder to e003
- F  //not available
- Gtimer; //count down timer for sending data to server  e.g. G300;  --> change countdown timer to 300 seconds
- I   //print information/Command
- Mmachinename;  //to record machine name with the sending data  e.g. Mmdb001;  --> change device to mdb001
- r   //reset setting to factory setting 
- z   //reset MCU
From the above setting the system will do as following:
1. esp32 will connect to SSID:TP-Link_5B9A, Password:97180937
2. Read electric parameters from phase 1 - 3 sequentially and store current, voltage,power factor and energy in three separate variables a1-a3,v1-v3,pf1=pf3,e1-e3 for phase 1 - 3
3. In every 300 seconds, data will be send url out to internet
3. url format:http://192.168.1.100/iot2024/e003/insert3E.php?devid=...&a1=..&v1=..&pf1=..&e1=..&a2=..&v2=..&pf2=..&e2=..&a3=..&v3=..&pf3=..&e3=..

In the server using php coding to extract data and store in mysql database. The php coding is following:

File: /var/www/html/iot2025/e003/insert3E.php 
<?php
/* http://192.168.1.100/iot2025/e003/insert3E.php?devid=e001&mcid=mdb001&a1=0.00&v1=236.80&pf1=0.00&e1=0.00&a2=0.00&v2=236.70&pf2=0.00&e2=0.00&a3=0.00&v3=237.00&pf3=0.00&e3=0.00 */
$devid = $_GET["devid"]; //device id
$mcID = $_GET["mcid"]; //machine id
$a1 = floatval($_GET["a1"]); //ampere phase 1
$v1 = floatval($_GET["v1"]); //voltage phase 1
$pf1 = floatval($_GET["pf1"]); //power factor phase 1
$e1 = floatval($_GET["e1"]); //energy phase 1

$a2 = floatval($_GET["a2"]); //ampere phase 2
$v2 = floatval($_GET["v2"]); //voltage phase 2
$pf2 = floatval($_GET["pf2"]); //power factor phase 2
$e2 = floatval($_GET["e2"]); //energy phase 2

$a3 = floatval($_GET["a3"]); //ampere phase 3
$v3 = floatval($_GET["v3"]); //voltage phase 3
$pf3 = floatval($_GET["pf3"]); //power factor phase 3
$e3 = floatval($_GET["e3"]); //energy phase 3

$devidStr = "'".$devid."'";
$mcIDStr = "'".$mcID."'";

$servername = "192.168.1.100"; //server ip address 
$username = "username"; //username mysql
$password = "password"; //password mysql

$conn = new mysqli($servername, $username, $password); //connection to mysql

$sql = "INSERT INTO $devid.electric3p(devID,mcid,a1,v1,pf1,e1,a2,v2,pf2,e2,a3,v3,pf3,e3) VALUES ($devidStr,$mcIDStr,$a1,$v1,$pf1,$e1,$a2,$v2,$pf2,$e2,$a3,$v3,$pf3,$e3)
if ($conn->query($sql) === TRUE) {
        echo "New record created successfully";
} else {
        echo "Error: " . $sql . "<br>" . $conn->error;
}
$conn->close();
?>

