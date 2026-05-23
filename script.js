const prefersReducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

const revealObserver = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting) {
      entry.target.classList.add("visible");
      revealObserver.unobserve(entry.target);
    }
  });
}, { threshold: 0.14 });

document.querySelectorAll(".reveal").forEach((element) => {
  if (prefersReducedMotion) {
    element.classList.add("visible");
  } else {
    revealObserver.observe(element);
  }
});

const navLinks = [...document.querySelectorAll(".nav-links a")];
const navTargets = navLinks
  .map((link) => document.querySelector(link.getAttribute("href")))
  .filter(Boolean);

const navObserver = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (!entry.isIntersecting) return;
    navLinks.forEach((link) => {
      link.classList.toggle("active", link.getAttribute("href") === `#${entry.target.id}`);
    });
  });
}, { rootMargin: "-42% 0px -52% 0px" });

navTargets.forEach((target) => navObserver.observe(target));

const heroScheduler = document.getElementById("heroScheduler");
const heroSchedulerState = document.getElementById("heroSchedulerState");
const heroCores = [...document.querySelectorAll("#heroChip .core")];
const heroTraceLines = [...document.querySelectorAll("#heroTrace span")];
const heroSteps = [
  { core: "T1", trace: 0, status: "LOAD" },
  { core: "WAIT", trace: 1, status: "INTERRUPT" },
  { core: "T2", trace: 0, status: "RESUME" },
  { core: "T3", trace: 2, status: "STALE_STORE" }
];
let heroStepIndex = 0;
let heroPaused = false;

function renderHeroScheduler() {
  const step = heroSteps[heroStepIndex % heroSteps.length];
  heroCores.forEach((core) => core.classList.toggle("active", core.dataset.core === step.core));
  heroTraceLines.forEach((line, index) => line.classList.toggle("active", index === step.trace));
  heroSchedulerState.textContent = heroPaused ? "PAUSED" : step.status;
  heroStepIndex += 1;
}

if (!prefersReducedMotion) {
  window.setInterval(() => {
    if (!heroPaused) renderHeroScheduler();
  }, 1200);
}

heroScheduler.addEventListener("click", () => {
  heroPaused = !heroPaused;
  heroSchedulerState.textContent = heroPaused ? "PAUSED" : "AUTO_TRACE";
});

heroScheduler.addEventListener("keydown", (event) => {
  if (event.key === "Enter" || event.key === " ") {
    event.preventDefault();
    heroScheduler.click();
  }
});

renderHeroScheduler();

const memoryLabels = {
  shared: "shared",
  heap: "heap",
  stack: "stack"
};

document.querySelectorAll("[data-memory]").forEach((segment) => {
  segment.addEventListener("pointerenter", () => {
    const key = memoryLabels[segment.dataset.memory];
    document.querySelectorAll("[data-memory]").forEach((item) => {
      item.classList.toggle("is-focused", item.dataset.memory === segment.dataset.memory);
    });
    document.querySelectorAll("[data-note]").forEach((note) => {
      note.classList.toggle("active", note.dataset.note === key);
    });
  });
});

document.getElementById("addressSpace").addEventListener("pointerleave", () => {
  document.querySelectorAll("[data-memory]").forEach((item) => item.classList.remove("is-focused"));
  document.querySelectorAll("[data-note]").forEach((note) => note.classList.toggle("active", note.dataset.note === "shared"));
});

const schedulerStates = [
  { lane: 0, text: "Thread_A executing quantum.", ready: false, wait: false },
  { lane: 1, text: "Dispatcher memilih Thread_B dari ready queue.", ready: true, wait: false },
  { lane: 2, text: "Thread_B menunggu I/O dan masuk wait queue.", ready: false, wait: true },
  { lane: 3, text: "Thread_C mendapat giliran berikutnya.", ready: false, wait: false }
];
let schedulerIndex = 0;

function renderScheduler() {
  const state = schedulerStates[schedulerIndex % schedulerStates.length];
  document.getElementById("schedulerStatus").textContent = state.text;
  document.querySelectorAll("#schedulerGantt span").forEach((lane, index) => {
    lane.classList.toggle("active", index === state.lane);
  });
  const readyToken = document.querySelector('[data-scheduler-token="ready"]');
  const waitToken = document.querySelector('[data-scheduler-token="wait"]');
  readyToken.classList.toggle("in-flight", state.ready);
  readyToken.classList.toggle("sleeping", state.wait);
  waitToken.classList.toggle("in-flight", state.wait);
  waitToken.textContent = state.wait ? "Thread_B [204]" : "I/O kosong";
  schedulerIndex += 1;
}

if (!prefersReducedMotion) {
  window.setInterval(renderScheduler, 1600);
}

renderScheduler();

const raceLines = [
  "> Inisialisasi Thread A & B...",
  "> Target eksekusi: 2 operasi increment",
  "[Thread_A] Membaca counter: 100",
  "[Thread_A] R1 = 101",
  "*** INTERRUPT: Timer Expired ***",
  "[Thread_B] Membaca counter: 100 // stale read",
  "[Thread_B] R2 = 101",
  "[Thread_B] Menulis counter: 101",
  "[Thread_A] Resume eksekusi...",
  "[Thread_A] Menulis counter: 101 // overwrite",
  "> Expected: 102",
  "> Actual: 101 (KORUP)"
];

const raceSequence = [
  { step: "a1", reg: "regA", value: "R1: 100", terminal: 2 },
  { step: "a2", reg: "regA", value: "R1: 101", terminal: 3 },
  { step: "as", terminal: 4, pauseA: true },
  { step: "b1", reg: "regB", value: "R2: 100", terminal: 5 },
  { step: "b2", reg: "regB", value: "R2: 101", terminal: 6 },
  { step: "b3", counter: "101", terminal: 7 },
  { step: "a3", counter: "101", corrupt: true, terminal: 9 }
];

let raceTimerIds = [];
let raceRunning = false;
let raceStepIndex = 0;

function clearRaceTimers() {
  raceTimerIds.forEach((id) => window.clearTimeout(id));
  raceTimerIds = [];
}

function resetRace() {
  clearRaceTimers();
  raceRunning = false;
  raceStepIndex = 0;
  document.querySelectorAll(".thread-code p").forEach((line) => line.classList.remove("active"));
  document.getElementById("threadA").style.opacity = "1";
  document.getElementById("threadB").style.opacity = "1";
  document.getElementById("regA").textContent = "R1: --";
  document.getElementById("regB").textContent = "R2: --";
  const counter = document.getElementById("sharedCounter");
  counter.textContent = "100";
  counter.classList.remove("corrupt");
  document.querySelector("#raceTerminal .terminal-output").innerHTML = "";
  document.getElementById("runRace").textContent = "> RUN_SIMULATION";
  document.getElementById("stepRace").textContent = "> NEXT_STEP";
}

function appendTerminalLine(index) {
  const terminal = document.querySelector("#raceTerminal .terminal-output");
  const div = document.createElement("div");
  div.textContent = raceLines[index];
  if (raceLines[index].includes("***") || raceLines[index].includes("KORUP") || raceLines[index].includes("stale")) {
    div.className = "error";
  }
  terminal.appendChild(div);
}

function applyRaceItem(item) {
  document.querySelectorAll(".thread-code p").forEach((line) => line.classList.remove("active"));
  document.querySelector(`[data-step="${item.step}"]`).classList.add("active");

  if (item.reg) document.getElementById(item.reg).textContent = item.value;
  if (item.pauseA) document.getElementById("threadA").style.opacity = "0.48";
  if (item.step === "b1") document.getElementById("threadB").style.opacity = "1";
  if (item.step === "a3") document.getElementById("threadA").style.opacity = "1";
  if (item.counter) document.getElementById("sharedCounter").textContent = item.counter;
  if (item.corrupt) document.getElementById("sharedCounter").classList.add("corrupt");
  appendTerminalLine(item.terminal);
}

function finishRace() {
  appendTerminalLine(10);
  appendTerminalLine(11);
  raceRunning = false;
  raceStepIndex = 0;
}

function runRace() {
  if (raceRunning) {
    resetRace();
    return;
  }

  resetRace();
  raceRunning = true;
  document.getElementById("runRace").textContent = "> RESET_SIMULATION";
  appendTerminalLine(0);
  appendTerminalLine(1);

  raceSequence.forEach((item, index) => {
    const timer = window.setTimeout(() => {
      applyRaceItem(item);

      if (index === raceSequence.length - 1) {
        window.setTimeout(finishRace, 450);
      }
    }, 760 * (index + 1));
    raceTimerIds.push(timer);
  });
}

document.getElementById("runRace").addEventListener("click", runRace);
document.getElementById("stepRace").addEventListener("click", () => {
  if (raceRunning) resetRace();
  if (raceStepIndex === 0) {
    resetRace();
    appendTerminalLine(0);
    appendTerminalLine(1);
    document.getElementById("stepRace").textContent = "> NEXT_STEP";
  }

  applyRaceItem(raceSequence[raceStepIndex]);
  raceStepIndex += 1;

  if (raceStepIndex >= raceSequence.length) {
    finishRace();
    document.getElementById("stepRace").textContent = "> RESTART_STEPS";
  }
});

const raceAutoObserver = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting && !raceRunning) {
      runRace();
      raceAutoObserver.disconnect();
    }
  });
}, { threshold: 0.45 });

raceAutoObserver.observe(document.getElementById("race"));

const collisionButton = document.getElementById("runCollision");
collisionButton.addEventListener("click", () => {
  const a = document.getElementById("collideA");
  const b = document.getElementById("collideB");
  const active = collisionButton.dataset.active === "true";
  const collisionStage = collisionButton.closest(".collision-stage");
  const collisionStatus = document.getElementById("collisionStatus");

  collisionButton.dataset.active = active ? "false" : "true";
  collisionButton.textContent = active ? "> TEST_COLLISION" : "> RESET";
  a.style.transform = active ? "translateX(0)" : "translateX(72%)";
  b.style.transform = active ? "translateX(0)" : "translateX(72%)";
  a.style.borderColor = active ? "" : "var(--danger)";
  b.style.borderColor = active ? "" : "var(--danger)";
  collisionStage.classList.toggle("violation", !active);
  collisionStatus.textContent = active ? "1 thread allowed" : "VIOLATION: 2 threads entered";
});

const mutexButton = document.getElementById("toggleMutex");
mutexButton.addEventListener("click", () => {
  const locked = mutexButton.dataset.locked === "true";
  mutexButton.dataset.locked = locked ? "false" : "true";
  mutexButton.textContent = locked ? "> TOGGLE_LOCK" : "> UNLOCK";

  document.getElementById("flowAcquire").classList.toggle("locked", !locked);
  document.getElementById("flowExecute").classList.toggle("locked", !locked);
  document.getElementById("lockIcon").textContent = locked ? "lock_open" : "lock";
  document.getElementById("mutexLane").classList.toggle("locked", !locked);
});

const perfObserver = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting) {
      entry.target.classList.add("in-view");
    }
  });
}, { threshold: 0.35 });

perfObserver.observe(document.getElementById("perfChart"));

const bufferCells = [...document.querySelectorAll("#bufferCells span")];
let bufferCount = 1;

window.setInterval(() => {
  bufferCount = (bufferCount + 1) % (bufferCells.length + 1);
  bufferCells.forEach((cell, index) => {
    const filled = index < bufferCount;
    cell.classList.toggle("filled", filled);
    cell.textContent = filled ? `D${index + 1}` : "";
  });
  document.getElementById("emptyCount").textContent = bufferCells.length - bufferCount;
  document.getElementById("fullCount").textContent = bufferCount;
  document.getElementById("mutexCount").textContent = bufferCount % 2 === 0 ? "0" : "1";
}, 1500);

const terminalTabs = {
  nosync: `$ gcc race.c -o race -lpthread
$ ./race

Expected Counter : 2000000
Actual Counter   : 1451823
Data Lost        : 548177
status           : DATA CORRUPTION DETECTED`,
  mutex: `$ gcc mutex.c -o mutex -lpthread
$ ./mutex

Expected Counter : 2000000
Actual Counter   : 2000000
Data Lost        : 0
status           : SAFE, SERIALIZED CRITICAL SECTION`,
  semaphore: `$ gcc producer_consumer.c -o pc -lpthread
$ ./pc

[PRODUCER] wait(empty), wait(mutex)
[BUFFER]   write item D1
[CONSUMER] wait(full), wait(mutex)
status     : IPC COORDINATED`
};

function setTerminalTab(key) {
  document.querySelectorAll(".tab").forEach((tab) => {
    tab.classList.toggle("active", tab.dataset.tab === key);
  });
  document.getElementById("terminalContent").textContent = terminalTabs[key];
}

document.querySelectorAll(".tab").forEach((tab) => {
  tab.addEventListener("click", () => setTerminalTab(tab.dataset.tab));
});

setTerminalTab("nosync");
