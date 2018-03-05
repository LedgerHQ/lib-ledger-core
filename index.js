let binding = null
function loadBinding() {
  if (!binding) {
    binding = require('bindings')('ledgerapp_nodejs') // eslint-disable-line global-require
  }
}

loadBinding()
