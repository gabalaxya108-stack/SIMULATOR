const expressionInput = document.getElementById('expression');
const validateBtn = document.getElementById('validateBtn');
const generateBtn = document.getElementById('generateBtn');
const resetBtn = document.getElementById('resetBtn');
const messageBox = document.getElementById('messageBox');
const variableSection = document.getElementById('variableSection');
const variableInputs = document.getElementById('variableInputs');
const results = document.getElementById('results');
const postfixValue = document.getElementById('postfixValue');
const variablesValue = document.getElementById('variablesValue');
const formatCards = document.getElementById('formatCards');
const comparisonTableBody = document.getElementById('comparisonTableBody');

function showMessage(message, type) {
  messageBox.className = `alert alert-${type}`;
  messageBox.textContent = message;
  messageBox.classList.remove('d-none');
}

function hideMessage() {
  messageBox.className = 'alert d-none';
  messageBox.textContent = '';
}

function resetForm() {
  expressionInput.value = '';
  variableSection.classList.add('d-none');
  variableInputs.innerHTML = '';
  results.classList.add('d-none');
  hideMessage();
}

function collectVariableValues() {
  const values = {};
  document.querySelectorAll('[data-variable-input]').forEach((input) => {
    const name = input.dataset.variableInput;
    const value = Number(input.value);
    values[name] = Number.isNaN(value) ? 0 : value;
  });
  return values;
}

function renderVariableInputs(variables) {
  variableInputs.innerHTML = '';
  if (!variables.length) {
    variableSection.classList.add('d-none');
    return;
  }
  variableSection.classList.remove('d-none');
  variables.forEach((variable) => {
    const col = document.createElement('div');
    col.className = 'col-md-3';
    col.innerHTML = `
      <label class="form-label fw-semibold">${variable}</label>
      <input type="number" class="form-control" data-variable-input="${variable}" value="1" min="0" />
    `;
    variableInputs.appendChild(col);
  });
}

function renderResults(data) {
  if (!data.success) {
    showMessage(data.error, 'danger');
    results.classList.add('d-none');
    return;
  }

  postfixValue.textContent = data.postfix || '-';
  variablesValue.textContent = data.variables.join(', ') || '-';
  formatCards.innerHTML = '';

  data.formats.forEach((format) => {
    const card = document.createElement('div');
    card.className = 'col-12';
    card.innerHTML = `
      <div class="card border-0 shadow-sm h-100">
        <div class="card-body">
          <div class="d-flex justify-content-between align-items-center mb-3">
            <h3 class="h5 mb-0">${format.name}</h3>
            <span class="badge bg-primary">${format.instructionCount} instructions</span>
          </div>
          <div class="mb-3">
            <strong>Instructions</strong>
            <pre class="bg-light p-3 rounded mt-2">${format.instructions.join('\n')}</pre>
          </div>
          <div class="border rounded p-3">
            <h4 class="h6">Step-by-step simulation</h4>
            ${format.steps.map((step) => `
              <div class="border rounded p-3 mb-2">
                <p class="mb-1"><strong>${step.instruction}</strong></p>
                <p class="mb-1 text-primary">${step.result}</p>
                <small class="text-muted">${step.state.map((entry) => `${entry.name}=${entry.value}`).join(', ')}</small>
              </div>
            `).join('')}
          </div>
          <div class="mt-3">
            <p class="mb-1"><strong>Registers / Stack:</strong> ${format.registers}</p>
            <p class="mb-1"><strong>Temporary Variables:</strong> ${format.temporaryVariables.join(', ')}</p>
            <p class="mb-0"><strong>Final Result:</strong> ${format.finalResult}</p>
          </div>
        </div>
      </div>
    `;
    formatCards.appendChild(card);
  });

  comparisonTableBody.innerHTML = '';
  data.comparisonRows.forEach((row) => {
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${row.name}</td><td>${row.value}</td><td>${row.name === 'Three Address' ? 'None' : row.name === 'Two Address' ? 'R1' : row.name === 'One Address' ? 'AC' : 'Stack'}</td><td>${row.name === 'Three Address' ? 't1, t2, ...' : 't1, t2, ...'}</td><td>${row.name === 'Three Address' ? 'Intermediate' : row.name === 'Two Address' ? 'R1 register' : row.name === 'One Address' ? 'AC register' : 'Stack top'}</td>`;
    comparisonTableBody.appendChild(tr);
  });

  results.classList.remove('d-none');
  showMessage('Expression validated and simulated successfully.', 'success');
}

async function processExpression() {
  const expression = expressionInput.value.trim();
  if (!expression) {
    showMessage('Please enter an expression.', 'danger');
    return;
  }
  const values = collectVariableValues();
  const query = new URLSearchParams({ expression, values: Object.entries(values).map(([key, value]) => `${key}:${value}`).join(',') }).toString();
  try {
    const response = await fetch(`http://127.0.0.1:8080/api/validate?${query}`);
    const data = await response.json();
    if (data.success) {
      renderVariableInputs(data.variables);
      renderResults(data);
    } else {
      renderVariableInputs([]);
      results.classList.add('d-none');
      showMessage(data.error, 'danger');
    }
  } catch (error) {
    showMessage('Unable to connect to the simulator server.', 'danger');
  }
}

validateBtn.addEventListener('click', processExpression);
generateBtn.addEventListener('click', processExpression);
resetBtn.addEventListener('click', resetForm);
