const API_URL = "http://localhost:3000/sensor";

async function loadData() {
  let currentPage = 1;
  const limit = 50;

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
