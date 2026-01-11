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

    // Calcular la escala entre el tamaño visual y el tamaño interno del canvas
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;

    // Ajustar las coordenadas del mouse a las coordenadas del canvas interno
    const canvasX = x * scaleX;
    const canvasY = y * scaleY;

    // Calcular posición en el grid usando cellSize real
    const gridX = Math.floor(canvasX / cellSize);
    const gridY = Math.floor(canvasY / cellSize);

    console.log(`Click: mouse(${x.toFixed(0)}, ${y.toFixed(0)}) -> canvas(${canvasX.toFixed(0)}, ${canvasY.toFixed(0)}) -> grid(${gridX}, ${gridY})`);

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
    console.log(`✅ Estableciendo meta en grid: (${gridX}, ${gridY})`);
    setGoal(gridX, gridY);
}

// ============================================
// Controls
// ============================================
function initControls() {
    // Pause button
    const pauseBtn = document.getElementById('pauseButton');
    if (pauseBtn) {
        pauseBtn.addEventListener('click', () => {
            console.log('Pause button clicked');
            // You can implement pause logic here if needed
        });
    }

    // Reset button
    const resetBtn = document.getElementById('resetButton');
    if (resetBtn) {
        resetBtn.addEventListener('click', async () => {
            console.log('Reset button clicked');
            await resetRobot();
        });
    }
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

async function resetRobot() {
    try {
        const response = await fetch(API_BASE + '/api/reset', {
            method: 'POST'
        });

        if (response.ok) {
            console.log('✅ Sistema reiniciado - Robot reposicionado');
            fetchState(); // Actualizar inmediatamente
        } else {
            console.error('Error al reiniciar');
        }
    } catch (error) {
        console.error('Error resetting robot:', error);
    }
}

// ============================================
// Statistics Dashboard
// ============================================
function initStatsChart() {
    // Chart.js not included, skip initialization
    return;
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
    // Stats UI elements don't exist in current HTML
    // Just log for debugging
    if (stats) {
        console.log('Stats received:', stats);
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

        const gridSizeEl = document.getElementById('gridSize');
        if (gridSizeEl) {
            gridSizeEl.textContent = `Visualización ${gridWidth}x${gridHeight}`;
        }

        // Resize canvas si cambió el tamaño del grid
        if (oldWidth !== gridWidth || oldHeight !== gridHeight) {
            resizeCanvas();
        }
    }

    // Update robot percentage display
    const robotsPercentEl = document.getElementById('robotsPercent');
    if (robotsPercentEl && state.robots && state.robots.length > 0) {
        robotsPercentEl.textContent = `${state.robots.length} activo(s)`;
    }

    // Update positions if we have robot data
    if (state.robots && state.robots.length > 0) {
        const robot = state.robots[0];
        const startPosEl = document.getElementById('startPos');
        const endPosEl = document.getElementById('endPos');
        const obstaclesAvoidedEl = document.getElementById('obstaclesAvoided');

        if (startPosEl) {
            startPosEl.textContent = `(${robot.x}, ${robot.y})`;
        }

        if (endPosEl && state.goal) {
            endPosEl.textContent = `(${state.goal.x}, ${state.goal.y})`;
        }

        if (obstaclesAvoidedEl) {
            obstaclesAvoidedEl.textContent = robot.obstaclesAvoided || 0;
        }
    }
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

    // Colors based on cell type - Soft/Feminine theme
    // 0 = EMPTY, 1 = OBSTACLE
    switch (type) {
        case 0: // EMPTY
            ctx.fillStyle = '#383a4a'; // Lighter gray for visibility
            break;
        case 1: // OBSTACLE
            ctx.fillStyle = '#d15278'; // Soft pink
            break;
        default:
            ctx.fillStyle = '#383a4a';
    }

    ctx.fillRect(px, py, cellSize, cellSize);
}

function renderGoal(x, y) {
    const px = x * cellSize + cellSize / 2;
    const py = y * cellSize + cellSize / 2;
    const radius = cellSize * 0.4;

    // Draw soft pink circle for goal
    ctx.fillStyle = '#e97ba0';
    ctx.beginPath();
    ctx.arc(px, py, radius, 0, Math.PI * 2);
    ctx.fill();

    // Soft glow outline
    ctx.strokeStyle = '#f5a5c0';
    ctx.lineWidth = 2;
    ctx.stroke();
}

function renderRobot(x, y, id) {
    const px = x * cellSize + cellSize / 2;
    const py = y * cellSize + cellSize / 2;
    const radius = cellSize * 0.35;

    // Draw soft pink/purple circle for robot
    ctx.fillStyle = '#f5a5c0';
    ctx.beginPath();
    ctx.arc(px, py, radius, 0, Math.PI * 2);
    ctx.fill();

    // Soft outline
    ctx.strokeStyle = '#ffc9dc';
    ctx.lineWidth = 2;
    ctx.stroke();

    // Robot ID (if cell is large enough)
    if (cellSize > 15) {
        ctx.fillStyle = '#1a1a26';
        ctx.font = `bold ${Math.floor(cellSize * 0.4)}px Arial`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('R', px, py);
    }
}

function drawGridLines() {
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.08)';
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
