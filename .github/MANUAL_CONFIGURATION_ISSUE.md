# Issue: Configuración Manual del Repositorio

**Título**: Configuración Manual del Repositorio - Verificación Open Source

**Etiquetas**: `configuration`, `setup`, `documentation`

---

## Descripción

Este issue documenta la configuración manual necesaria para completar la configuración Open Source del repositorio `cayennelpp_decoder`. 

**Instrucciones**: Marca cada checkbox cuando completes la configuración correspondiente.

---

## 🔒 Protección de Rama Principal (main)

**Ubicación**: Settings → Branches → Add branch protection rule → Branch name pattern: `main`

- [ ] **Require a pull request before merging** 
  - Require approvals: **1** (del propietario)
- [ ] **Dismiss stale pull request approvals** when new commits are pushed
- [ ] **Require status checks to pass before merging** 
  - Require branches to be up to date before merging
- [ ] Seleccionar status checks cuando estén disponibles:
  - Build and Test
  - Format Check
- [ ] **Require conversation resolution before merging**
- [ ] **Do not allow bypassing the above settings**
- [ ] **Block force pushes** (debe estar habilitado por defecto)
- [ ] **Allow deletions** - DESHABILITADO (protege contra eliminación accidental)
- [ ] ❌ **NO marcar** "Require signed commits" (específicamente excluido según requisitos)

---

## 🛡️ Seguridad

**Ubicación**: Settings → Security → Code security and analysis

- [ ] Habilitar **Dependabot alerts**
- [ ] Habilitar **Dependabot security updates** (opcional pero recomendado)

---

## ⚙️ Features del Repositorio

**Ubicación**: Settings → General → Features

- [ ] Habilitar **Issues**
- [ ] Habilitar **Discussions**

---

## 📝 Configuración de Pull Requests

**Ubicación**: Settings → General → Pull Requests

- [ ] **Allow merge commits** - HABILITADO (política de resolución de conflictos)
- [ ] **Allow auto-merge** - DESHABILITADO (específicamente deshabilitado según requisitos)
- [ ] **Automatically delete head branches** - HABILITADO (recomendado)

---

## 💬 Configuración de Discussions

**Ubicación**: Settings → Discussions (después de habilitar la feature)

Crear las siguientes categorías:

- [ ] **General** (💬) - Discusiones generales
- [ ] **Q&A** (🙏) - Configurar con formato Q&A para preguntas y respuestas
- [ ] **Ideas / Feature Requests** (💡) - Ideas y solicitudes de características
- [ ] **Announcements** (📣) - Anuncios del proyecto

---

## ✅ Verificación Final

Verificar que todo está configurado correctamente:

- [ ] La rama `main` aparece en Settings → Branches con un icono de protección
- [ ] No se pueden hacer push directos a `main` (requiere PR)
- [ ] Los PRs requieren aprobación del propietario antes de merge
- [ ] Dependabot alerts está habilitado
- [ ] Issues está habilitado
- [ ] Discussions está habilitado
- [ ] Auto-merge está deshabilitado
- [ ] Los templates de issues aparecen al crear un nuevo issue (Bug Report, Feature Request)
- [ ] El workflow de CI se ejecuta correctamente en los PRs

---

## 📚 Referencias

- [GitHub Branch Protection](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
- [Dependabot Alerts](https://docs.github.com/en/code-security/dependabot/dependabot-alerts/about-dependabot-alerts)
- [GitHub Discussions](https://docs.github.com/en/discussions)
- Ver `CONTRIBUTING.md` para la guía completa de contribución

---

## 📝 Notas

_Añade aquí cualquier nota, problema encontrado o configuración adicional realizada durante el proceso._

---

**Creado el**: 2026-02-11  
**Tiempo estimado**: 10-15 minutos  
**Prioridad**: Alta
