// ============================================
// RideBot Web Interface - Main JavaScript
// ============================================

// Configuración
const API_BASE = '';
const UPDATE_INTERVAL = 200; // ms

// Estado global
let state = {
    grid: null,
    robots: [],
    goal: { x: 0, y: 0 },
    paused: false,
    speed: 200,
    lastUpdate: null
};

// Canvas y rendering
let canvas, ctx;
let cellSize = 10;
let gridWidth = 60;
let gridHeight = 40;

// Modo de edición
let editMode = false;

// Stats dashboard
let statsChart = null;
let statsHistory = {
    labels: [],
    completed: [],
    maxPoints: 20
};

// Inicialización
document.addEventListener('DOMContentLoaded', () => {
    initCanvas();
    initControls();
    initStatsChart();
    startPolling();
});

// ============================================
// Canvas Setup
// ============================================
function initCanvas() {
    canvas = document.getElementById('gridCanvas');
    ctx = canvas.getContext('2d');

    // Tamaño inicial - se ajustará cuando llegue el estado
    canvas.width = 800;
    canvas.height = 800;

    // Canvas click para establecer objetivo
    canvas.addEventListener('click', handleCanvasClick);
}

// Ajustar tamaño del canvas según el grid
function resizeCanvas() {
    if (!state.grid) return;

    const maxSize = 800; // Tamaño máximo del canvas
    const aspectRatio = gridWidth / gridHeight;

    if (aspectRatio >= 1) {
        // Grid más ancho que alto
        canvas.width = maxSize;
        canvas.height = maxSize / aspectRatio;
    } else {
        // Grid más alto que ancho
        canvas.height = maxSize;
        canvas.width = maxSize * aspectRatio;
    }

    // Renderizar después de resize
    renderGrid();
}

function handleCanvasClick(event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    // Calcular posición en el grid usando cellSize real
    const gridX = Math.floor(x / cellSize);
    const gridY = Math.floor(y / cellSize);

    // Validar que esté dentro del grid
    if (gridX < 0 || gridX >= gridWidth || gridY < 0 || gridY >= gridHeight) {
        console.warn('Click fuera del grid');
        return;
    }

    // Si está en modo edición, toggle obstáculo
    if (editMode) {
        toggleObstacle(gridX, gridY);
        return;
    }

    // Validar que no sea un obstáculo
    if (state.grid && state.grid.cells[gridY][gridX] === 1) {
        console.warn(`No se puede colocar objetivo en obstáculo en (${gridX}, ${gridY})`);
        alert('❌ No puedes colocar el objetivo en un obstáculo');
        return;
    }

    // Establecer como objetivo
    console.log(`Click en grid: (${gridX}, ${gridY})`);
    setGoal(gridX, gridY);
}

// ============================================
// Controls
// ============================================
function initControls() {
    // Goal button
    const setGoalBtn = document.getElementById('setGoalBtn');
    setGoalBtn.addEventListener('click', () => {
        const x = parseInt(document.getElementById('goalX').value);
        const y = parseInt(document.getElementById('goalY').value);
        setGoal(x, y);
    });

    // Edit mode button
    const editModeBtn = document.getElementById('editModeBtn');
    const editorControls = document.getElementById('editorControls');

    editModeBtn.addEventListener('click', () => {
        editMode = !editMode;
        if (editMode) {
            editModeBtn.textContent = '✅ Modo Edición Activo';
            editModeBtn.classList.add('active');
            editorControls.style.display = 'block';
            canvas.style.cursor = 'crosshair';
        } else {
            editModeBtn.textContent = '🖊️ Activar Modo Edición';
            editModeBtn.classList.remove('active');
            editorControls.style.display = 'none';
            canvas.style.cursor = 'pointer';
        }
    });

    // Clear obstacles button
    const clearObstaclesBtn = document.getElementById('clearObstaclesBtn');
    clearObstaclesBtn.addEventListener('click', clearAllObstacles);

    // Random obstacles button
    const randomObstaclesBtn = document.getElementById('randomObstaclesBtn');
    randomObstaclesBtn.addEventListener('click', () => generateRandomObstacles(25));
}

// ============================================
// API Calls
// ============================================
async function fetchState() {
    try {
        const response = await fetch(API_BASE + '/api/state');
        if (!response.ok) throw new Error('Failed to fetch state');

        const data = await response.json();
        state = data;
        updateUI();
        renderGrid();
    } catch (error) {
        console.error('Error fetching state:', error);
    }
}

async function setGoal(x, y) {
    try {
        const response = await fetch(API_BASE + '/api/goal', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ x, y })
        });

        if (response.ok) {
            document.getElementById('goalX').value = x;
            document.getElementById('goalY').value = y;
            console.log(`Objetivo establecido en (${x}, ${y})`);
        }
    } catch (error) {
        console.error('Error setting goal:', error);
    }
}

async function toggleObstacle(x, y) {
    console.log(`[DEBUG] Intentando toggle obstáculo en (${x}, ${y})`);
    try {
        const response = await fetch(API_BASE + '/api/obstacle', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ x, y })
        });

        const data = await response.json();
        console.log('[DEBUG] Respuesta del servidor:', data);

        if (response.ok && data.success) {
            const action = data.action === 'added' ? 'agregado' : 'eliminado';
            console.log(`✅ Obstáculo ${action} en (${x}, ${y})`);

            // Forzar actualización inmediata
            fetchState();
        } else {
            console.warn('No se pudo modificar el obstáculo');
        }
    } catch (error) {
        console.error('Error toggling obstacle:', error);
    }
}

async function clearAllObstacles() {
    if (!confirm('¿Seguro que quieres limpiar todos los obstáculos?')) {
        return;
    }

    try {
        await fetch(API_BASE + '/api/clear-obstacles', {
            method: 'POST'
        });
        console.log('✅ Obstáculos limpiados');
        fetchState(); // Actualizar inmediatamente
    } catch (error) {
        console.error('Error clearing obstacles:', error);
    }
}

async function generateRandomObstacles(percentage) {
    try {
        await fetch(API_BASE + '/api/random-obstacles', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ percentage })
        });
        console.log(`✅ Obstáculos generados (${percentage}%)`);
        fetchState(); // Actualizar inmediatamente
    } catch (error) {
        console.error('Error generating obstacles:', error);
    }
}

// ============================================
// Statistics Dashboard
// ============================================
function initStatsChart() {
    const ctx = document.getElementById('statsChart');
    if (!ctx) return;

    statsChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Tareas Completadas',
                data: [],
                borderColor: '#10b981',
                backgroundColor: 'rgba(16, 185, 129, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        stepSize: 1
                    }
                }
            },
            animation: {
                duration: 750
            }
        }
    });
}

async function fetchStats() {
    try {
        const response = await fetch(API_BASE + '/api/stats');
        if (response.ok) {
            return await response.json();
        }
    } catch (error) {
        console.error('Error fetching stats:', error);
    }
    return null;
}

function updateStatsUI(stats) {
    if (!stats) return;

    // Update stat cards
    document.getElementById('statCompletedTasks').textContent = stats.completedTasks || 0;
    document.getElementById('statCellsTraveled').textContent = stats.cellsTraveled || 0;
    document.getElementById('statDistance').textContent =
        ((stats.totalDistance || 0) / 1000).toFixed(2) + ' km';
    document.getElementById('statEfficiency').textContent =
        (stats.efficiency || 0).toFixed(1) + '%';

    // Update additional stats
    document.getElementById('statRobots').textContent =
        `${stats.robotsActive || 0}/${stats.totalRobots || 0}`;

    // Format uptime
    const uptime = stats.uptime || 0;
    const minutes = Math.floor(uptime / 60);
    const seconds = uptime % 60;
    document.getElementById('statUptime').textContent =
        minutes > 0 ? `${minutes}m ${seconds}s` : `${seconds}s`;

    // Update chart
    if (statsChart) {
        const now = new Date();
        const timeLabel = now.toLocaleTimeString('es-ES', {
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });

        // Add new data point
        statsHistory.labels.push(timeLabel);
        statsHistory.completed.push(stats.completedTasks || 0);

        // Keep only last maxPoints
        if (statsHistory.labels.length > statsHistory.maxPoints) {
            statsHistory.labels.shift();
            statsHistory.completed.shift();
        }

        // Update chart data
        statsChart.data.labels = statsHistory.labels;
        statsChart.data.datasets[0].data = statsHistory.completed;
        statsChart.update('none'); // Update without animation for smoother real-time
    }
}

// ============================================
// UI Updates
// ============================================
function updateUI() {
    // Update grid size
    if (state.grid) {
        const oldWidth = gridWidth;
        const oldHeight = gridHeight;

        gridWidth = state.grid.width;
        gridHeight = state.grid.height;

        document.getElementById('gridSize').textContent =
            `Grid: ${gridWidth}x${gridHeight}`;
        document.getElementById('currentGridSize').textContent =
            `${gridWidth}x${gridHeight}`;

        // Resize canvas si cambió el tamaño del grid
        if (oldWidth !== gridWidth || oldHeight !== gridHeight) {
            resizeCanvas();
        }
    }

    // Update robot count
    document.getElementById('robotCount').textContent = state.robots.length;

    // Update last update time
    const now = new Date();
    const timeStr = now.toLocaleTimeString();
    document.getElementById('lastUpdate').textContent =
        `Última actualización: ${timeStr}`;
}

// ============================================
// Grid Rendering
// ============================================
function renderGrid() {
    if (!state.grid) return;

    const w = canvas.width;
    const h = canvas.height;

    // Clear canvas
    ctx.clearRect(0, 0, w, h);

    // Calculate cell size
    cellSize = Math.min(w / gridWidth, h / gridHeight);

    // Render cells
    const cells = state.grid.cells;
    for (let y = 0; y < gridHeight; y++) {
        for (let x = 0; x < gridWidth; x++) {
            const cellType = cells[y][x];
            renderCell(x, y, cellType);
        }
    }

    // Render goal
    if (state.goal) {
        renderGoal(state.goal.x, state.goal.y);
    }

    // Render robots
    for (const robot of state.robots) {
        renderRobot(robot.x, robot.y, robot.id);
    }

    // Draw grid lines
    drawGridLines();
}

function renderCell(x, y, type) {
    const px = x * cellSize;
    const py = y * cellSize;

    // Colors based on cell type
    // 0 = EMPTY, 1 = OBSTACLE
    switch (type) {
        case 0: // EMPTY
            ctx.fillStyle = '#f8fafc';
            break;
        case 1: // OBSTACLE
            ctx.fillStyle = '#334155';
            break;
        default:
            ctx.fillStyle = '#f8fafc';
    }

    ctx.fillRect(px, py, cellSize, cellSize);
}

function renderGoal(x, y) {
    const px = x * cellSize + cellSize / 2;
    const py = y * cellSize + cellSize / 2;
    const radius = cellSize * 0.4;

    // Draw star shape for goal
    ctx.fillStyle = '#10b981';
    ctx.beginPath();
    ctx.arc(px, py, radius, 0, Math.PI * 2);
    ctx.fill();

    // Outline
    ctx.strokeStyle = '#059669';
    ctx.lineWidth = 2;
    ctx.stroke();
}

function renderRobot(x, y, id) {
    const px = x * cellSize + cellSize / 2;
    const py = y * cellSize + cellSize / 2;
    const radius = cellSize * 0.35;

    // Draw circle for robot
    ctx.fillStyle = '#3b82f6';
    ctx.beginPath();
    ctx.arc(px, py, radius, 0, Math.PI * 2);
    ctx.fill();

    // Outline
    ctx.strokeStyle = '#1e40af';
    ctx.lineWidth = 2;
    ctx.stroke();

    // Robot ID (if cell is large enough)
    if (cellSize > 15) {
        ctx.fillStyle = 'white';
        ctx.font = `bold ${Math.floor(cellSize * 0.4)}px Arial`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('R', px, py);
    }
}

function drawGridLines() {
    ctx.strokeStyle = '#e2e8f0';
    ctx.lineWidth = 0.5;

    // Vertical lines
    for (let x = 0; x <= gridWidth; x++) {
        const px = x * cellSize;
        ctx.beginPath();
        ctx.moveTo(px, 0);
        ctx.lineTo(px, gridHeight * cellSize);
        ctx.stroke();
    }

    // Horizontal lines
    for (let y = 0; y <= gridHeight; y++) {
        const py = y * cellSize;
        ctx.beginPath();
        ctx.moveTo(0, py);
        ctx.lineTo(gridWidth * cellSize, py);
        ctx.stroke();
    }
}

// ============================================
// Polling
// ============================================
async function startPolling() {
    async function poll() {
        await fetchState();
        const stats = await fetchStats();
        updateStatsUI(stats);
    }

    // Initial fetch
    poll();

    // Poll every 200ms
    setInterval(poll, 200);
}

console.log('RideBot Web Interface initialized');
