import { execFileSync } from 'node:child_process';
import { readdirSync } from 'node:fs';
import { join } from 'node:path';
import { binary, projectRoot } from './compiler.mjs';

const sources = readdirSync(join(projectRoot, 'src')).filter(name => name.endsWith('.c')).map(name => join(projectRoot, 'src', name));
try {
  execFileSync('gcc', ['-std=c11', '-Wall', '-Wextra', ...sources, '-o', binary], { cwd: projectRoot, stdio: 'inherit', windowsHide: true });
  console.log(`Built ${binary}`);
} catch (error) {
  console.error(error.code === 'ENOENT' ? 'GCC was not found. Install GCC/MinGW and add it to PATH.' : 'Compiler build failed. See GCC diagnostics above.');
  process.exitCode = 1;
}
