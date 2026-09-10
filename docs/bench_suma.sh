#!/usr/bin/env bash
# Benchmark de la Suma de Riemann paralela (OpenMP).
# Compila, hace el barrido de hilos y calcula speedup y eficiencia.
# Uso:  bash docs/bench_suma.sh
set -e

SRC="paralelo/suma_omp.c"
BIN="/tmp/suma_omp_bench"
A=0
B=3.141592653589793
HILOS=(1 2 4 8 12)   # ajusta segun los cores de tu maquina
REPS=3               # corridas por config; se toma la mejor

# --- Compilar (detecta Mac con libomp o Linux con gcc) ---
if command -v gcc >/dev/null && gcc -fopenmp -xc -E - </dev/null >/dev/null 2>&1; then
  gcc -fopenmp -O2 "$SRC" -o "$BIN"
else
  BREW=$(brew --prefix libomp)
  clang -Xpreprocessor -fopenmp -I"$BREW/include" -L"$BREW/lib" -lomp -O2 "$SRC" -o "$BIN"
fi

# --- Barrido ---
declare -a T
echo "hilos  tiempo(s)"
for p in "${HILOS[@]}"; do
  best=""
  for _ in $(seq "$REPS"); do
    t=$("$BIN" "$A" "$B" "$p" | grep Tiempo | grep -oE '[0-9]+\.[0-9]+')
    if [ -z "$best" ]; then best=$t; else best=$(python3 -c "print(min($best,$t))"); fi
  done
  T[$p]=$best
  printf "%-6d %s\n" "$p" "$best"
done

# --- Speedup y eficiencia ---
echo ""
printf "%-6s %-12s %-10s %-12s\n" "hilos" "tiempo(s)" "speedup" "eficiencia(%)"
T1=${T[1]}
for p in "${HILOS[@]}"; do
  python3 -c "s=$T1/${T[$p]}; print('%-6d %-12.4f %-10.2f %-12.1f' % ($p, ${T[$p]}, s, 100*s/$p))"
done
