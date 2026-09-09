# WeatherPro

WeatherPro es un complemento de información del clima para [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) que soporta múltiples fuentes de datos y opciones de personalización avanzadas.

El repositorio original del proyecto es [Haojia521/TrafficMonitorPlugins](https://github.com/Haojia521/TrafficMonitorPlugins), publicado hasta la versión v0.14 y posteriormente archivado. Este repositorio reescribió y actualizó completamente el código, publicando la versión v1.0. Las nuevas versiones se publicarán en este repositorio.

Se recomienda usar junto a TrafficMonitor v1.86 o superior.

> **Nota**: Esta es una versión traducida al español que, además, sustituye la identificación de la API de `weather.com.cn` por `api.weather.com` e incorpora una nueva fuente de datos: **Open-Meteo** (https://open-meteo.com/), una API gratuita y sin necesidad de registro.

## Características

- Soporta múltiples fuentes de datos (consulte la [guía de fuentes de datos](docs/DataSourceGuide.md) para la configuración)
  - Weather.com (weather.com)
  - QWeather (qweather.com)
  - OpenWeather (openweathermap.org)
  - Open-Meteo (open-meteo.com) — **gratis, sin API key ni registro**
- Soporta mostrar información del clima en dos líneas cuando la barra de tareas está en horizontal
- Soporta configurar una zona de datos fijos
- Soporta desplazamiento de texto largo en la ventana principal de ancho fijo
- Soporta amplias opciones de personalización de la información meteorológica

## Novedades de la versión V1.2.0

- [Mejora] Iconos integrados de múltiples resoluciones para adaptarse a distintas pantallas DPI
- [Mejora] La interfaz de selección de ubicación muestra las coordenadas geográficas de las ubicaciones candidatas
- [Mejora] Comprobación activa de actualizaciones del complemento
- [Mejora] Ventana de progreso no modal durante la comprobación de actualizaciones, actualización manual del clima y búsqueda de ubicaciones
- [Corrección] Faltaban algunos códigos de fenómenos meteorológicos de weather.com

## Descripción de la interfaz

- Zona principal de datos en la barra de tareas

  Muestra el clima y la temperatura; el clima puede renderizarse como icono o texto. La nueva versión soporta ancho fijo con desplazamiento de texto largo. Los datos pueden ser: clima actual, clima de hoy, pronóstico de 24~48 horas o de 48~72 horas.

  ![Zona principal](images/taskbar-wnd.png)

  V1.1 añade el modo de doble línea. Si la barra de tareas está horizontal, el número de elementos de TrafficMonitor es impar y el elemento principal de WeatherPro está al final, puede ocupar dos líneas para dibujar el icono y el texto.

  ![Modo doble línea](images/taskbar-wnd-dual-line-mode.png)

  V1.0 soporta configurar una zona de información fija que muestra datos según la franja horaria y el elemento seleccionados. Tras modificar la configuración de la zona fija, debe reiniciar TrafficMonitor para que los cambios surtan efecto.

  ![Zona fija](images/taskbar-wnd-pinned-items.png)

- Ventana emergente de información al pasar el ratón

  ![Tooltip](images/tooltip-info.png)

- Interfaz de configuración

  Configura la fuente de datos, ubicación y modo de visualización. Cuando se publica una nueva versión, se mostrará un botón "¡Nueva versión!" en la parte inferior para guiarle en la descarga.

  ![Configuración](images/main-settings.png)

- Interfaz de configuración de API

  Opciones de la API de weather.com.

  ![Opciones WCC](images/api-wcc-options.png)

  Opciones de la API de QWeather (qweather.com).

  ![Opciones QW](images/api-qweather-options.png)

  Opciones de la API de OpenWeather (openweathermap.org).

  ![Opciones OW](images/api-openweather-options.png)

- Interfaz de selección de ubicación

  Además de buscar por texto, se soporta la búsqueda por coordenadas. Las coordenadas son obligatorias para algunas APIs, como QWeather (calidad del aire) y OpenWeather.

  ![Selección de ubicación](images/set-location.png)

- Configuración de zona fija

  Configura la zona fija por franja horaria y elemento de datos. El orden y las etiquetas de la zona fija pueden modificarse en la página de configuración de la barra de tareas de TrafficMonitor.

  ![Configuración zona fija](images/pinned-items-settings.png)

- Configuración de ubicación automática

  Seleccione libremente el método de auto-ubicación. Si todos los métodos fallan, no se cambiará la información de ubicación actual.

  ![Configuración auto-ubicación](images/auto-loc-settings.png)

### Historial de versiones

- V1.0.4

  - [Nuevo] Soporte de fuente de datos OpenWeather
  - [Nuevo] Establecer ubicación por coordenadas geográficas
  - [Nuevo] Desplazamiento de texto largo en la zona principal
  - [Nuevo] Zona de datos fijos
  - [Nuevo] Punto de notificación en el icono cuando hay alertas
  - [Nuevo] Aviso de nueva versión
  - [Mejora] Auto-ubicación soporta API, sistema operativo, coordenadas IP y nombre de región IP
  - [Mejora] Visor de alertas y logs en ventana separada
  - [Corrección] Idioma del hilo no se configuraba correctamente al consultar el clima
  - [Corrección] La API de QWeather devolvía un JWT en caché tras cambiar la clave

- V1.1.0

  - [Nuevo] Modo de doble línea. En barra de tareas horizontal, si el elemento principal está solo a la derecha, puede usar doble línea
  - [Nuevo] Opción para mostrar las coordenadas geográficas en el resumen del clima
  - [Corrección] La marca de tiempo del log era UTC en vez de hora local
  - [Corrección] No se usaba la nueva interfaz del complemento para configurar el idioma
  - [Corrección] Inconsistencia en los tipos de marca de tiempo de QWeather
  - [Corrección] Posible cuelgue al analizar cadenas JSON

### Cambios en esta versión en español

- [Nuevo] Toda la interfaz y los mensajes traducidos al español
- [Nuevo] Fuente de datos Open-Meteo (sin API key, sin registro, gratuita)
- [Cambio] Renombrado el identificador `api_weather.com.cn` → `api_weather.com` (compatible con configuraciones anteriores)
- [Cambio] La localización por defecto para sistemas no chinos ahora es español
