// variable untuk menyimpan alamat broker hivemq
const brokerUrl = 'wss://broker.hivemq.com:8884/mqtt';

// membuat client id
const options = {
    clientId: 'webDashboard_'+ Math.random().toString(16).substring(2,8)
};

// menyimpan id status agar bisa merubah online menjadi connecting....
const status_server = document.getElementById('status_server');

// merubah online menjadi connecting.....
status_server.innerText = "Connecting.....";

// menghubungkan dengan mqtt
const client = mqtt.connect(brokerUrl,options);

// mendapatkan id dari checkbox
const checkBox = document.getElementById('relay1');

// kita tambahkana listener
checkBox.addEventListener('change',function(){
    if(checkBox.checked){
        client.publish('iot/esp32/relay',"ON");
        console.log("berhasil mengirimkan sinyal nyala");
    }
    else{
        client.publish('iot/esp32/relay',"OFF");
        console.log("berhasil mengirimkan sinyal mati");
    }
        
});

// function ketika mqtt sudah berhasil terkoneksikan
client.on('connect',function(){
    console.log("Berhasil terhubung dengan mqtt");
    status_server.innerText = "Online";
    status_server.style.color = "#2ecc71"; //mengganti warna status server
    // subscribe ke mqtt 
    client.subscribe('iot/esp32/dht11/suhu',function(error){
        if(!error){
            console.log("MQTT berhasil terkoneksikan dengan suhu!!!!");
        }
        else{
            console.log("Tidak bisa terkoneksi karena :" + error);
        }
    });
    client.subscribe('iot/esp32/dht11/hum',function(error){
        if(!error){
            console.log("MQTT berhasil terkoneksikan dengan kelembapan!!!!");
        }
        else{
            console.log("Tidak bisa terkoneksi karena :" + error);
        }
    });
});

// mendengarkan pesan masuk: ini jalan ketika ada data masuk ke topik yang kita subscribe
client.on('message',function(topik,message){
    // merubah karena massage itu byte kita ubah jadi string
    var pesanMasuk = message.toString();
    console.log("Pesan diterima dari topik " + topik + ":" + pesanMasuk);

    // merubah tampilan html berdasarkan pesan yang masuk dari mqtt
    var elementSuhu = document.getElementById("suhu");
    var elementKelembapan = document.getElementById("kelembapan");
    if (topik === 'iot/esp32/dht11/suhu') {
        elementSuhu.innerText = pesanMasuk;
    }
    else if(topik === 'iot/esp32/dht11/hum'){
        elementKelembapan.innerText = pesanMasuk;
    }
});

var indikasiSuhu = document.getElementById('suhu');



