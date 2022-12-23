const express = require('express');
const mqtt = require('mqtt');
const app = express();
const port = 3000;

const mqttHost = 'mqtt.eclipseprojects.io';
const mqttPort = 1883;
const clientID = 'mqttjs_' + Math.random().toString(16).slice(2, 8);

const mqttConnectUrl = `mqtt://${mqttHost}:${mqttPort}`;
const mqttClient = mqtt.connect(mqttConnectUrl, { clientId: clientID });

const topic = 'irrigacao_acionada';

mqttClient.on('connect', () => {
    console.log('Cliente MQTT conectado.');
    mqttClient.subscribe([topic], () => {
        console.log(`Cliente MQTT inscrito no tópico ${topic}.`);
    });
});

mqttClient.on('message', (topic, message) => {
    console.log(`Cliente MQTT recebeu mensagem no tópico ${topic}: ${message.toString()}`);
});

app.listen(port, () => {
    console.log(`Servidor Web escutando na porta ${port}`);
});