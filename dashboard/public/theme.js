// Applies the saved colour theme before the page is painted. Light is the default.
try {
  if (localStorage.getItem('theme') === 'dark') document.documentElement.dataset.theme = 'dark';
} catch {}
