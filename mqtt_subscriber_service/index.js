const mqtt = require('mqtt');
const { Pool } = require('pg');

// PostgreSQL connection
const pool = new Pool({
  user: 'mainflux',
  host: 'mqtt-db',
  database: 'subscriptions',
  password: 'mainflux',
  port: 5432,
});

// MQTT client connection (replace with your MQTT broker URL)
const client = mqtt.connect('mqtt://mosquitto:1884');

// Subscribe to a single topic for both temperature and humidity
client.on('connect', () => {
  console.log('Connected to MQTT broker');
  client.subscribe('esp8266/sensor_data');
});

// Handle incoming MQTT messages
client.on('message', async (topic, message) => {
  const msg = message.toString();

  // Parse the JSON message
  let data;
  try {
    data = JSON.parse(msg);
  } catch (error) {
    console.error('Error parsing JSON:', error);
    return;
  }

  const temperature = parseFloat(data.temperature);
  const humidity = parseFloat(data.humidity);
  const deviceName = 'ESP8266'; // Tên thiết bị cố định hoặc trích xuất từ thông điệp nếu có

  // Log the data (bạn có thể lưu vào database hoặc chỉ log ra console)
  try {
    console.log('Data received:', { deviceName, temperature, humidity });

    // Example of inserting into PostgreSQL
    await pool.query(
      'INSERT INTO sensor_data (device_name, nhietdo, doam, time) VALUES ($1, $2, $3, NOW())',
      [deviceName, temperature, humidity]
    );
    console.log('Data saved to PostgreSQL:', { deviceName, temperature, humidity });
  } catch (err) {
    console.error('Error saving data to PostgreSQL:', err);
  }
});
