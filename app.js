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
// ini untuk mendapat id canvas
const canvasSuhu = document.getElementById('grafikSuhu');
var indikasiSuhu = document.getElementById('suhu');

// untuk membuat chart
const myChart = new Chart(canvasSuhu,{
    type: "line",
    data:{
        labels: [],
        datasets: [
        {
            label: "Suhu (C)",
            data: [],
            borderColor: "rgba(255, 99, 132, 1)",//warna untuk suhu
        },
        {
            label: "Kelembapan (%)",
            data: [],
            borderColor: "rgba(54, 162, 235, 1)",//warna untuk suhu
        }]
    },
    options: {
        maintainAspectRatio: false // INI DIA KUNCINYA
    }
})

// kita tambahkana listener
checkBox.addEventListener('change',function(){
    // mendapatkan id dari status menyala or tidaknya
    var gantiStatusDevice = document.getElementById('infoDevice');
    if(checkBox.checked){
        client.publish('iot/esp32/relay',"ON");
        console.log("berhasil mengirimkan sinyal nyala");
        gantiStatusDevice.style.color = "#27ae60";
        gantiStatusDevice.innerText = "Status: Nyala";
    }
    else{
        client.publish('iot/esp32/relay',"OFF");
        console.log("berhasil mengirimkan sinyal mati");
        gantiStatusDevice.style.color = "#bd2424";
        gantiStatusDevice.innerText = "Status: Mati";
    }
    
});

// function ketika mqtt sudah berhasil terkoneksikan
client.on('connect',function(){
    console.log("Berhasil terhubung dengan mqtt");
    status_server.innerText = "Online";
    status_server.style.color = "#2ecc71"; //mengganti warna status server
    // subscribe ke mqtt 
    client.subscribe('iot/esp32/dht11/data',function(error){
        if(!error){
            console.log("MQTT berhasil terkoneksikan dengan suhu dan kelembapan");
        }
        else{
            console.log("Tidak bisa terkoneksi karena :" + error);
        }
    });
    // subscribe ke status relay
    client.subscribe('iot/esp32/relay/status');
    // publish ke topik status relay agar ketika web di refresh bisa langsung mendapatkan status relay
    client.publish('iot/esp32/relay/status','REQUEST');
});

// mendengarkan pesan masuk: ini jalan ketika ada data masuk ke topik yang kita subscribe
client.on('message',function(topik,message){
    // mengambil object javaScript untuk waktu(jam,menit,detik)
    const sekarang = new Date();
    // merubah karena massage itu byte kita ubah jadi string
    var pesanMasuk = message.toString();
    const jam = sekarang.getHours();
    const detik = sekarang.getSeconds();
    const menit = sekarang.getMinutes();
    let waktuSekarang = jam + ":" + menit + ":" + detik;
    console.log("Pesan diterima dari topik " + topik + ":" + pesanMasuk);
    // merubah tampilan html berdasarkan pesan yang masuk dari mqtt
    var elementSuhu = document.getElementById("suhu");
    var elementKelembapan = document.getElementById("kelembapan");
    if (topik === 'iot/esp32/dht11/data') {
        // mengambil data json menggunakan object ini
        let dataSensor = JSON.parse(pesanMasuk);
        // merubah suhu maupun kelembapan di html
        elementSuhu.innerText = dataSensor.suhu;
        elementKelembapan.innerText = dataSensor.hum;
        // menambahkan data ke chart
        let suhuMasuk = parseFloat(dataSensor.suhu);
        let kelembapanMasuk = parseFloat(dataSensor.hum);
        myChart.data.datasets[0].data.push(suhuMasuk);
        myChart.data.datasets[1].data.push(kelembapanMasuk);
        myChart.data.labels.push(waktuSekarang);
    }

    // LOGIC AGAR TIDAK ADA BUG ON OFF
    if(topik === 'iot/esp32/relay/status'){
        var gantiStatusDevice = document.getElementById('infoDevice');
        if(pesanMasuk === "ON"){
            checkBox.checked = true;
            gantiStatusDevice.style.color = "#2ecc71";
            gantiStatusDevice.innerText = "Status: Nyala";
        }
        else if(pesanMasuk === "OFF"){
            checkBox.checked = false;
            gantiStatusDevice.style.color = "#bd2424";
            gantiStatusDevice.innerText = "Status: Mati";
        }
    }
    if(myChart.data.labels.length > 10){
        myChart.data.datasets[0].data.shift();
        myChart.data.datasets[1].data.shift();
        myChart.data.labels.shift();
    }
        myChart.update();
});

