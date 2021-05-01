var mysql = require('mysql');
var mqtt = require('mqtt');

//CREDENCIALES MYSQL
var con = mysql.createConnection({
    host: "termocaseta.tk",
    user: "admin_bd_termo",
    password: "termo@0521",
    database: "admin_bd_termo"
});

//CREDENCIALES MQTT
var options = {
    port: 1883,
    host: 'host_servicio_mqtt',         // Sustituir por el host adecuado
    clientId: 'access_termo_' + Math.round(Math.random() * (0- 10000) * -1) ,
    username: 'user_mqtt',              // Sustituir por las credenciales oportunas
    password: 'pwd_mqtt',               // Sustituir por las credenciales oportunas
    keepalive: 60,
    reconnectPeriod: 1000,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    encoding: 'utf8'
};

var client = mqtt.connect("mqtt://host_mqtt", options);     // Sustituir por la url del servicio mqtt

var texto_espera = "Funcionando"

// Función para completar la inserción de valores
function inserta(frase_sql, sensor, concepto, valor, hora){
    frase_sql += "'" + sensor + "', " + concepto + ", " + valor + ", '" + hora + "');";
    //console.log(frase_sql);
    //hacemos la consulta para insertar....
    con.query(frase_sql, function (err, result, fields) {
        if (err) throw err;
        console.log("Fila insertada correctamente");
    });
}

// Conexión a la base de datos
con.connect(function(err){
    if (err) throw err;
    console.log("Conexión a MYSQL exitosa!!!");
});

// Conexión al broker MQTT
client.on('connect', function () {
    console.log("Conexión  MQTT Exitosa!!");

    client.subscribe('panel/#', function (err) {
        console.log("Suscripción a 'panel/#' exitosa!")
    });
})

//CUANDO SE RECIBE MENSAJE
client.on('message', function (topic, message) {
    console.log("Mensaje recibido desde -> " + topic + ". Mensaje -> " + message.toString());
    var topicos = topic.split("/");

    if (topicos[0] == "panel" && topicos[1] == "valores"){
        console.log("Recepción de tensión en panel solar");
        var msg = message.toString(); 
        console.log(msg);
        var msg_split = msg.split(";");
        // El mensaje viene con la forma: valor; fecha
        var inicio_sql = "INSERT INTO `admin_bd_termo`.`panelsolar` (`panelsolar_tension`, `panelsolar_hora`) "
        inicio_sql += "VALUES (";
        var frase_sql = msg_split[0] + ",'" + msg_split[1] + "')";
        frase_sql = inicio_sql + frase_sql;
        console.log(frase_sql);

        //hacemos la consulta para insertar....
        con.query(frase_sql, function (err, result, fields) {
            if (err) throw err;
            console.log("Fila insertada correctamente");
        });

        texto_espera = "Funcionando"
    }
});

// Para mostrar que el sistema sigue funcionando e impedir que se cierre la base de datos
setInterval(function () {
    var query ='SELECT 1 + 1 as result';
    texto_espera += "."
    if (texto_espera.length>15) {
        texto_espera = "Funcionando";
    }

    con.query(query, function (err, result, fields) {
        if (err) throw err;
        console.log(texto_espera);
    });
}, 5000);
