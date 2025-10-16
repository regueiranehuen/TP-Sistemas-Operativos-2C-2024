# TP Sistemas Operativos Segundo Cuatrimestre 2024: The Last of C

* Consiste en un sistema de 4 módulos en los que cada uno simula las funciones de componentes de un sistema operativo: Kernel, CPU, Memoria y Filesystem.
Estas simulaciones se realizan a partir de pseudocódigos que ejecutan los hilos pertenecientes a los procesos planificados por Kernel.

Ejemplo de uso:
![FIBO_47](FIBO_47.mp4)

* En el ejemplo, se puede obvservar que tras la ejecución de los hilos planificados por Kernel, en AX quedó almacenado el número 46 de la sucesión de Fibonacci (1836311903), y en BX el número 47 de la misma (2971215073), por lo cual se llegó al objetivo que tenía el pseudocódigo ingresado en Kernel.
* Tras la instrucción DUMP_MEMORY, se creó el archivo dump correspondiente al proceso que llevó a cabo la ejecución del pseudocódigo.
* Las especificaciones que indican detalles como tamaños de procesos, algoritmos de planificación y tamaños de bloques se especificaron en los configs de cada módulo.
* Así como en este ejemplo la ejecución fue con un único proceso y mediante colas multinivel como algoritmo de planificación, debido a los configs se pueden elegir múltiples formas de ejecución



--------------
# tp-scaffold

Esta es una plantilla de proyecto diseñada para generar un TP de Sistemas
Operativos de la UTN FRBA.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint, ejemplo: `checkpoint-1`.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

> [!WARNING]
> Asegúrense de que el código compila y cumple con los requisitos del checkpoint
> antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=filesystem "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

## Guías útiles

- [Cómo interpretar errores de compilación](https://docs.utnso.com.ar/primeros-pasos/primer-proyecto-c#errores-de-compilacion)
- [Cómo utilizar el debugger](https://docs.utnso.com.ar/guias/herramientas/debugger)
- [Cómo configuramos Visual Studio Code](https://docs.utnso.com.ar/guias/herramientas/code)
- **[Guía de despliegue de TP](https://docs.utnso.com.ar/guías/herramientas/deploy)**

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy
