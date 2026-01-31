#!/bin/bash

# Script de utilidad para trabajar con el proyecto Cayene Decoder
# Uso: ./cayene.sh [comando]

set -e  # Salir si hay errores

PROJECT_ROOT="/home/pablo/Desktop/muicr/rsa/proyecto/cayene_decoder"
BUILD_TYPE="${BUILD_TYPE:-release}"  # Por defecto release

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

function print_usage() {
    echo "Uso: $0 [comando]"
    echo ""
    echo "Comandos disponibles:"
    echo "  configure [debug|release]  - Configurar el proyecto con CMake"
    echo "  build [debug|release]      - Compilar el proyecto"
    echo "  test [debug|release]       - Ejecutar tests"
    echo "  run [debug|release]        - Ejecutar el ejemplo básico"
    echo "  clean                      - Limpiar archivos de build"
    echo "  rebuild [debug|release]    - Limpiar y recompilar"
    echo "  format                     - Formatear código con clang-format"
    echo "  tidy                       - Analizar código con clang-tidy"
    echo ""
    echo "Ejemplos:"
    echo "  $0 build release"
    echo "  $0 test"
    echo "  $0 run debug"
}

function configure_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Configurando proyecto (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    cmake --preset "$build_type"
    echo -e "${GREEN}✓ Configuración completa${NC}"
}

function build_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Compilando proyecto (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    cmake --build "build/$build_type"
    echo -e "${GREEN}✓ Compilación completa${NC}"
}

function test_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Ejecutando tests (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    ctest --preset "$build_type" --output-on-failure
}

function run_example() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Ejecutando ejemplo básico (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    "./build/$build_type/examples/basic_example"
}

function clean_project() {
    echo -e "${YELLOW}Limpiando archivos de build...${NC}"
    cd "$PROJECT_ROOT"
    rm -rf build
    echo -e "${GREEN}✓ Limpieza completa${NC}"
}

function rebuild_project() {
    local build_type="${1:-$BUILD_TYPE}"
    clean_project
    configure_project "$build_type"
    build_project "$build_type"
}

function format_code() {
    echo -e "${YELLOW}Formateando código...${NC}"
    cd "$PROJECT_ROOT"
    find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
    echo -e "${GREEN}✓ Código formateado${NC}"
}

function tidy_code() {
    echo -e "${YELLOW}Analizando código con clang-tidy...${NC}"
    cd "$PROJECT_ROOT"
    clang-tidy -p build/release src/*.cpp include/**/*.hpp
    echo -e "${GREEN}✓ Análisis completo${NC}"
}

# Main
if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

COMMAND=$1
shift

case $COMMAND in
    configure)
        configure_project "$@"
        ;;
    build)
        build_project "$@"
        ;;
    test)
        test_project "$@"
        ;;
    run)
        run_example "$@"
        ;;
    clean)
        clean_project
        ;;
    rebuild)
        rebuild_project "$@"
        ;;
    format)
        format_code
        ;;
    tidy)
        tidy_code
        ;;
    *)
        echo -e "${RED}Error: Comando desconocido '$COMMAND'${NC}"
        echo ""
        print_usage
        exit 1
        ;;
esac
