#!/usr/bin/env bash
# Compara la Suma de Riemann secuencial contra OpenMP.
# Uso: bash docs/bench_suma.sh
set -e

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SRC_SEQ="$ROOT/secuencial/suma.c"
SRC_OMP="$ROOT/paralelo/suma_omp.c"
BIN_SEQ="/tmp/suma_serial_bench"
BIN_OMP="/tmp/suma_omp_bench"
A=0
B=3.141592653589793
HILOS=(1 2 4 8)
REPS=3

# Compilar ambos programas con las mismas opciones.
if command -v gcc >/dev/null && gcc -fopenmp -xc -E - </dev/null >/dev/null 2>&1; then
  gcc -fopenmp -O2 "$SRC_SEQ" -o "$BIN_SEQ" -lm
  gcc -fopenmp -O2 "$SRC_OMP" -o "$BIN_OMP" -lm
else
  BREW=$(brew --prefix libomp)
  COMMON=(-Xpreprocessor -fopenmp -I"$BREW/include" -L"$BREW/lib" -lomp -O2)
  clang "${COMMON[@]}" "$SRC_SEQ" -o "$BIN_SEQ"
  clang "${COMMON[@]}" "$SRC_OMP" -o "$BIN_OMP"
fi

echo "=== SUMA DE RIEMANN: SECUENCIAL ==="
best_seq=""
for rep in $(seq "$REPS"); do
  t=$("$BIN_SEQ" "$A" "$B" | grep "Tiempo de ejecucion" | grep -oE '[0-9]+\.[0-9]+')
  printf "corrida %-2d %s s\n" "$rep" "$t"
  if [ -z "$best_seq" ]; then best_seq=$t; else best_seq=$(python3 -c "print(min($best_seq,$t))"); fi
done
printf "mejor secuencial: %s s\n\n" "$best_seq"

declare -a T
echo "=== SUMA DE RIEMANN: PARALELO OPENMP ==="
for p in "${HILOS[@]}"; do
  best=""
  echo "hilos: $p"
  for rep in $(seq "$REPS"); do
    t=$("$BIN_OMP" "$A" "$B" "$p" | grep "Tiempo de ejecucion" | grep -oE '[0-9]+\.[0-9]+')
    printf "  corrida %-2d %s s\n" "$rep" "$t"
    if [ -z "$best" ]; then best=$t; else best=$(python3 -c "print(min($best,$t))"); fi
  done
  T[$p]=$best
  printf "  mejor: %s s\n" "$best"
done

echo ""
echo "=== COMPARACION: SPEEDUP = Ts/Tp ==="
printf "%-12s %-12s %-10s %-14s\n" "version" "tiempo(s)" "speedup" "eficiencia(%)"
printf "%-12s %-12.6f %-10.2f %-14s\n" "secuencial" "$best_seq" 1 "-"
for p in "${HILOS[@]}"; do
  python3 -c "s=$best_seq/${T[$p]}; print('%-12s %-12.6f %-10.2f %-14.1f' % ('OpenMP-$p', ${T[$p]}, s, 100*s/$p))"
done
