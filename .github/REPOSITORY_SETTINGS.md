# Configuración del Repositorio - Estándares Open Source

Este documento describe la configuración necesaria para el repositorio `cayennelpp_decoder` siguiendo los estándares de proyectos Open Source.

## 🔒 Protección de Rama Principal (main)

La rama `main` debe estar protegida con las siguientes reglas:

### Configuración en GitHub (Settings → Branches → Branch protection rules)

**Regla para: `main`**

#### Protección contra cambios directos:
- ✅ **Require a pull request before merging**
  - ✅ Require approvals: **1** (del propietario)
  - ✅ Dismiss stale pull request approvals when new commits are pushed
  - ❌ Require review from Code Owners (opcional)

#### Verificaciones de estado:
- ✅ **Require status checks to pass before merging**
  - ✅ Require branches to be up to date before merging
  - Los checks específicos se configurarán cuando existan workflows de CI

#### Reglas adicionales:
- ✅ **Require conversation resolution before merging**
- ✅ **Require linear history** (opcional, para mantener historial limpio)
- ❌ **Do not require signed commits** (específicamente excluido)

#### Restricciones:
- ✅ **Do not allow bypassing the above settings**
- ✅ **Restrict pushes that create matching branches** (solo administradores)

#### Force Push y Eliminación:
- ✅ **Block force pushes** (habilitado por defecto)
- ✅ **Allow deletions** (deshabilitado - protege contra eliminación accidental)

## 🛡️ Seguridad

### Dependabot
Configurado mediante `.github/dependabot.yml`:
- ✅ Alertas de seguridad habilitadas
- ✅ Actualizaciones automáticas de dependencias

### Vulnerability Alerts
**Configuración en GitHub (Settings → Security → Vulnerability alerts):**
- ✅ **Dependabot alerts**: Enabled
- ✅ **Dependabot security updates**: Enabled (opcional)

### Security Policy
- ✅ Archivo `SECURITY.md` creado en la raíz del proyecto
- ✅ Define proceso de reporte de vulnerabilidades

## ⚙️ Configuración General del Repositorio

**Settings → General:**

### Features
- ✅ **Issues**: Enabled
  - Issue templates configurados en `.github/ISSUE_TEMPLATE/`
- ✅ **Discussions**: Enabled
  - Para preguntas de la comunidad y soporte
- ❌ **Projects**: Opcional
- ❌ **Wiki**: Opcional (usar documentación en README)

### Pull Requests
- ❌ **Allow merge commits**: Enabled (Política: Crear commit de merge)
- ❌ **Allow squash merging**: Opcional
- ❌ **Allow rebase merging**: Opcional
- ❌ **Always suggest updating pull request branches**: Enabled
- ❌ **Allow auto-merge**: **DISABLED** (específicamente deshabilitado)
- ❌ **Automatically delete head branches**: Enabled (recomendado)

### Merge Strategy
Según los requisitos:
- ✅ **Política de resolución de conflictos**: Crear commit de merge
  - Esto se logra dejando habilitado "Allow merge commits"
  - Los conflictos se resolverán manualmente creando commits de merge

## 📋 Archivos de Configuración Incluidos

### `.github/dependabot.yml`
Configuración de Dependabot para:
- Actualizaciones de GitHub Actions
- Alertas de vulnerabilidades

### `.github/ISSUE_TEMPLATE/`
Templates para issues:
- `bug_report.yml`: Reporte de bugs
- `feature_request.yml`: Solicitud de características
- `config.yml`: Configuración de issue templates

### `.github/pull_request_template.md`
Template para Pull Requests con checklist de revisión

### `SECURITY.md`
Política de seguridad del proyecto:
- Versiones soportadas
- Proceso de reporte de vulnerabilidades
- Características de seguridad habilitadas

## 🔧 Pasos de Configuración Manual en GitHub

**Estos pasos deben realizarse en la interfaz web de GitHub:**

1. **Ir a Settings → Branches**
   - Click en "Add branch protection rule"
   - Branch name pattern: `main`
   - Configurar todas las opciones mencionadas arriba

2. **Ir a Settings → Security → Code security and analysis**
   - Habilitar "Dependabot alerts"
   - Habilitar "Dependabot security updates" (opcional)

3. **Ir a Settings → General**
   - En "Features", habilitar "Issues" y "Discussions"
   - En "Pull Requests", deshabilitar "Allow auto-merge"
   - Configurar merge strategy como "Allow merge commits"

4. **Ir a Settings → Discussions**
   - Habilitar Discussions si no está habilitado
   - Configurar categorías apropiadas

## ✅ Verificación de Configuración

Para verificar que todo está configurado correctamente:

- [ ] La rama `main` aparece en Settings → Branches con protección activa
- [ ] No se pueden hacer push directos a `main` (requiere PR)
- [ ] Los PR requieren aprobación antes de merge
- [ ] Dependabot alerts está habilitado
- [ ] Issues está habilitado
- [ ] Discussions está habilitado
- [ ] Auto-merge está deshabilitado
- [ ] El archivo SECURITY.md es visible en el repositorio
- [ ] Los templates de issues aparecen al crear un nuevo issue

## 📚 Referencias

- [GitHub Branch Protection](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
- [Dependabot](https://docs.github.com/en/code-security/dependabot)
- [Security Policy](https://docs.github.com/en/code-security/getting-started/adding-a-security-policy-to-your-repository)
