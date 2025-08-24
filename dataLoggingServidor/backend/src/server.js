import express from "express";
import swaggerUI from "swagger-ui-express";
import swaggerDocument from "./docs/swagger.json" with { type: "json" };
import { sensorRouter } from "./routes/index.js";
import cors from "cors";


const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());

// Rotas
app.use("/sensor", sensorRouter);
app.use("/api-docs", swaggerUI.serve, swaggerUI.setup(swaggerDocument));

app.listen(port, () => {
  console.log(`🚀 API rodando em http://localhost:${port}`);
});
