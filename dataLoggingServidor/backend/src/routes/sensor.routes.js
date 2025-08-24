import { Router } from "express";
import { SensorController } from "../controllers/sensor.controllers.js";

const sensorRouter = Router();

sensorRouter.post("/", SensorController.post);
sensorRouter.get("/", SensorController.get);

export { sensorRouter };
