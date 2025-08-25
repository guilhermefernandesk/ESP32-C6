const API_URL = "http://localhost:3000/sensor";

const ctx = document.getElementById("sensorChart").getContext("2d");

// // cria o gráfico vazio
const chart = new Chart(ctx, {
  type: "line",
  data: {
    labels: [],
    datasets: [
      {
        label: "value1",
        data: [],
        borderColor: "blue",
        backgroundColor: "rgba(0, 0, 255, 0.15)",
        tension: 0.25,
        fill: true,
      },
      {
        label: "value2",
        data: [],
        borderColor: "red",
        backgroundColor: "rgba(255, 0, 0, 0.15)",
        tension: 0.25,
        fill: true,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: { ticks: { maxRotation: 0 } },
      y: { beginAtZero: false },
    },
    plugins: {
      legend: { display: true },
      tooltip: { mode: "index", intersect: false },
    },
  },
});

let currentPage = 1;
const limit = 10;

async function loadData() {
  try {
    const response = await fetch(`${API_URL}?page=${currentPage}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
      },
    });
    if (!response.ok) throw new Error("Erro ao buscar dados");

    const sensores = await response.json();
    const tbody = document.querySelector("#sensorTable tbody");
    tbody.innerHTML = ""; // limpa antes de preencher

    sensores.forEach((sensor) => {
      const tr = document.createElement("tr");
      tr.innerHTML = `
        <td>${sensor.id}</td>
        <td>${sensor.device_id}</td>
        <td>${sensor.sensor}</td>
        <td>${sensor.location}</td>
        <td>${sensor.value1}</td>
        <td>${sensor.value2}</td>
        <td>${new Date(sensor.reading_time).toLocaleString()}</td>
      `;
      tbody.appendChild(tr);
    });

    // ===== Gráfico =====
    const labels = sensores.map((s) => new Date(s.reading_time).toLocaleTimeString()).reverse();
    const values1 = sensores.map((s) => s.value1).reverse();
    const values2 = sensores.map((s) => s.value2).reverse();
    chart.data.labels = labels;
    chart.data.datasets[0].data = values1;
    chart.data.datasets[1].data = values2;
    chart.update();
  } catch (error) {
    console.error(error);
    alert("Não foi possível carregar os dados");
  }
}

// botões para navegação
document.querySelector("#prev").addEventListener("click", () => {
  if (currentPage > 1) {
    currentPage--;
    loadData(currentPage);
  }
});
document.querySelector("#next").addEventListener("click", () => {
  currentPage++;
  loadData(currentPage);
});

window.onload = loadData;
