import { pool } from "../database/index.js";

export class SensorController {
  static async get(req, res) {
    try {
      // pegar página e limite via query params
      const page = parseInt(req.query.page) || 1;
      const limit = parseInt(req.query.limit) || 50;
      const offset = (page - 1) * limit;
      const result = await pool.query("SELECT * FROM sensores ORDER BY reading_time DESC LIMIT $1 OFFSET $2", [limit, offset]);
      res.status(200).json(result.rows);
    } catch (error) {
      console.error(error);
      res.status(500).send("Erro ao processar dados");
    }
  }

  static async post(req, res) {
    try {
      const { device_id, sensor, location, value1, value2 } = req.body;
      console.log(device_id, sensor, location, value1, value2);
      if (!device_id || !sensor || !location || !value1 || !value2) {
        return res.status(400).send("Dados incompletos");
      }
      const result = await pool.query("INSERT INTO sensores (device_id, sensor, location, value1, value2) VALUES ($1, $2, $3, $4, $5) RETURNING *", [
        device_id,
        sensor,
        location,
        value1,
        value2,
      ]);
      res.status(201).json(result.rows[0]);
    } catch (error) {
      console.error(error);
      res.status(500).send("Erro ao processar dados");
    }
  }
}
