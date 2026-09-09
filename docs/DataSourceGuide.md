# Guía de configuración de fuentes de datos

## 1. Weather.com (weather.com)

Lista para usar sin configuración.

## 2. QWeather (www.qweather.com)

### 2.1 Crear cuenta

- Entre en la [página de desarrolladores de QWeather](https://dev.qweather.com/) y pulse en "Registro gratuito"
- Tras registrarse, entre en la [consola de desarrollo](https://console.qweather.com/home?lang=es)
- Pulse en "Configuración" en la barra lateral izquierda, copie el contenido de "API Host" y péguelo en el campo "Host de API" de las opciones de API del complemento

### 2.2 Crear proyecto

- Pulse en "Gestión de proyectos" en la barra lateral y, a continuación, en "Crear proyecto" en la esquina superior derecha
- Introduzca el nombre del proyecto y pulse "Guardar"

### 2.3 Crear credencial

- Pulse en "Crear credencial" en la esquina superior derecha e introduzca el nombre de la credencial

#### 2.3.1 Key

- En "Método de autenticación" seleccione "API KEY" y pulse guardar para generar la clave
- Copie el contenido de "API KEY", vaya a las opciones de API del complemento, seleccione "Key" y péguelo en el campo correspondiente

> Nota: A partir del 1 de enero de 2027, la autenticación mediante API KEY estará sujeta a límites de solicitudes.

#### 2.3.2 JWT

- Primero genere los archivos de clave con el complemento
  - En las opciones de API del complemento, seleccione "JWT"
  - Pulse "Crear clave" y seleccione una carpeta para guardar los archivos de clave
  - Pulse "Copiar clave pública" para copiar la clave pública al portapapeles del sistema
- Vuelva a la página web; en "Método de autenticación" seleccione "JSON Web Token"
- Pegue la clave pública en el campo "Subir clave pública" y pulse guardar
- En la página de credenciales JWT, copie "ID de credencial" y péguelo en el campo "ID de credencial" de las opciones de API del complemento
- En la página del proyecto, copie "ID de proyecto" y péguelo en el campo "ID de proyecto" de las opciones de API del complemento

## 3. OpenWeather (www.openweathermap.org)

Tras registrar una cuenta, copie la clave desde la página [API Keys](https://home.openweathermap.org/api_keys) al campo "Clave de API" de las opciones de API del complemento.

Con la cuota gratuita, puede consultar el clima actual, la calidad del aire y la ubicación geográfica.

Este complemento soporta la interfaz "One Call 3.0".

## 4. Open-Meteo (www.open-meteo.com) — **Recomendado**

**Gratuita, sin API key y sin registro.**

Open-Meteo es una API meteorológica abierta y gratuita que no requiere registro ni claves de API. Ofrece:

- Clima actual (temperatura, humedad, sensación térmica, viento, precipitación, código WMO)
- Pronóstico de 3 días (temperaturas máx/mín, código WMO, índice UV, probabilidad de precipitación, humedad)
- Calidad del aire en tiempo real (AQI estándar de EE.UU., PM2.5, PM10)
- Geocodificación directa (nombre → coordenadas) integrada con soporte de varios idiomas, incluido el español
- Geocodificación inversa (coordenadas → nombre) mediante el servicio gratuito BigDataCloud

### Uso

1. En la configuración del complemento, seleccione "Open-Meteo (open-meteo.com)" como fuente de datos
2. Pulse "Opciones de API" para elegir las unidades (métricas o imperiales)
3. Use "Establecer ubicación" para buscar su ciudad por nombre o coordenadas

No se necesita ninguna clave. La auto-ubicación funciona mediante el sistema operativo o por IP (a través de los servicios existentes).

### Limitaciones

- No hay alertas meteorológicas (Open-Meteo no proporciona este dato)
- Sin pronóstico horario detallado (se ofrece pronóstico diario de 3 días)
- Límite de ~10.000 solicitudes diarias para uso no comercial (suficiente para uso personal)

> Para uso comercial intensivo, consulte los términos de Open-Meteo en https://open-meteo.com/en/terms
