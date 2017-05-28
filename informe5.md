# ENTREGA INDIVIDUAL 5: SCHED

## swtch

1)*Explicar la siguiente línea de código en task_spawn():*
~~~
void *stack = stack_alloc(STACK_SIZE) - sizeof(struct TaskData);
~~~

Stack_alloc, para un stack de 8 MiB, "reserva" desde el tope la cantidad de bytes que se le pasen (en este caso STACK_SIZE) y actualiza su variable para que el nuevo "tope" sea la base de la región reservada, que es el valor devuelto. Luego se le "reserva" al base pointer recibido el tamaño de un TaskData, esto se hace para luego agregarlo y dejar el stack en el mismo estado que lo dejaria un llamado a swtch.

2)*diff de swtch*
~~~
 .globl swtch
 swtch:
+       pushl %ebp
+       mov %esp,%ebp   
+       pushfl
+       pusha
+       
+       mov 8(%ebp),%eax        #EAX=oldsp
+       mov %esp,(%eax)         #actualiza SP en old
+       mov 12(%ebp),%eax       #EAX=newsp
+       mov (%eax),%esp    		#SP=*newsp
+       
+       popa
+       popfl
+       popl %ebp
        ret
~~~

3)*¿Cómo se debería cambiar la implementación de swtch() si se eliminara de TaskData el elemento reg_ebp?*

Al principio y al final no se debe pushear/popear %ebp y en los mov se debe referenciar respecto de %esp, para lo cual hay que hacer las cuentas de cuántos bytes retrocedió por cada push hecho desde el llamado a swtch.
Además, el ebp salvado en pusha pasa a ser relevante, por lo que tampoco se puede hacer mov %esp,%ebp dado que lo pisa.

## sched

1)*Al ejecutarse los contadores ¿cuántos aparecen? ¿Cuáles? Si no aparecen tres, explicar qué está sucediendo, y aplicar una solución en la función correspondiente.*

Aparecen el naranja y el rojo. Lo que ocurre es que al salir del for "new" siempre tiene el valor del último READY. Para el primer ciclo todos estan READY y son guardados en el orden de llamadas a task_spawn, es decir verde-naranja-rojo. Luego new apunta al rojo. Cuando rojo hace su ciclo y llama a sched el ultimo READY es el naranja porque rojo esta RUNNING, luego salta al naranja y pone rojo como READY, y se resetea el ciclo, nunca llamando a verde.
La solución a esto depende de la política de scheduling a utilizar siempre y cuando cumpla que para t finito cualquier proceso READY va a llegar a RUNNING.

2)*diff de sched*

Nota: todavía sigue pasando por el idle task al recorrer Tasks en forma "circular", pero no preocupa mucho porque la consigna del punto *idle* es resolver ese problema. En este caso, la solución es agregar dos chequeos más.

~~~
void sched() {
     struct Task *new = 0;
     struct Task *old = current;

-    for (int i = 0; i < MAX_TASK; i++) {
-        if (Tasks[i].status == READY)
-            new = &Tasks[i];
-    }
+    for (struct Task* i = current+1;i!=current;i++){
+        if (i >= Tasks + MAX_TASK) i = Tasks;
+        if (i->status == READY){
+           new = i;
+           break;
+        }
     }
-
     if (new) {
         old->status = READY;  // XXX 🤔?
         current = new;
~~~

## ticks

1)*¿Cómo está configurado el delay de los timers? ¿Se están respetando las velocidades? Si las velocidades son incorrectas, explicar por qué está sucediendo, y cómo se podría arreglar en esta implementación (scheduling cooperativo) frente a otras implementaciones (scheduling con desalojo).*

El delay se implementa con while(contador < delay){}, luego las velocidades no se respetan porque el contador se resuelve muy rápido. Lo que se quiere es que el "delay" que recibe contador actúe como la inversa del tiempo que se le permite trabajar, luego una posible solución con scheduling cooperativo es llamar a sched en el while, lo cual es por cada DELAY(delay) veces que yieldea, trabaja una.
Para un preemptive scheduling el que debiera saber cuanto puede trabajar cada task es el scheduler, así sabe cuando desalojarlo, debido a que hacer un yield no asegura cambio a otro contador. Contando con un timer que cada quantum de tiempo lo despierte, el scheduler puede asignar estos intervalos según la velocidad que se le quiera dar al contador.

2)*diff de contador.c*
~~~
 void contador(unsigned char linea, char color, unsigned char delay) {
@@ -13,10 +13,10 @@ void contador(unsigned char linea, char color, unsigned char
 delay) {
 
         unsigned p = 0;
         unsigned long long i = 0;
-
-        while (i++ < DELAY(delay))
-            ;
-

         while (counter[p] == '9') {
             counter[p++] = '0';
         }
@@ -30,6 +30,8 @@ void contador(unsigned char linea, char color, unsigned char delay) {
             *buf++ = color;
         }
 
-        sched();
+               while (i++ < DELAY(delay)){
+            if (i % TICKS == 0) sched();
+               }
     }
 }
~~~

## exit

1)*Cambiar el ciclo de la función contador() para que ejecute un número constante de iteraciones (por ejemplo 500). ¿Qué ocurre según van terminando las tareas?*

Recordando que el task_spawn carga un bloque inicial de TaskData con retaddr = entry, el primer switch a ese task va a hacer pop y cargar en EIP la dirección pasada: en el caso de por ejemplo, contador1, va a pushear los argumentos y llamar a contador. El problema viene cuando vuelva de contador() y llame a ret, porque va a ir al stack (ya no del task) a buscar la próxima instrucción, lo cual no tiene sentido.
NOTA: Si chequeara permisos debería levantar un segfault, pero entiendo que no lo hace. ASDASDASDASDWADAWDAWD SIEMPRE TIRA 0x53f000ff !!!!!
# PREGUNTAR


2)*Implementar la función task_exit() e invocarla en contador() tras finalizar el ciclo principal. ¿Es suficiente para arreglar el problema? Si no: ¿qué más se debió arreglar?*

Es suficiente para la ejecución, ya que al marcarlo como free y hacer un context switch nunca más va a volver a ejecutarse. Sin embargo, la memoria reservada nunca se libera.

# PREGUNTAR

# QUIEN SE ENCARGA DE "LIBERAR" EL STACK? EL KERNEL? COMO LE MARCO ESO? PARA ESO SERVIA EL STATUS ZOMBIE @LINUX?

3)*diff de contador.c*
~~~
+#define CANT_ITER 50
 
 void contador(unsigned char linea, char color, unsigned char delay) {
     char counter[COUNTLEN] = {'0'};  // Our ASCII digit counter (RTL).
-
-    while (1) {
+       
+    unsigned int iteraciones = CANT_ITER;
+    while (iteraciones--) {
         char *buf = (void *) 0xb8000 + linea * 160;
         char *c = &counter[COUNTLEN];
 
@@ -34,4 +36,6 @@ void contador(unsigned char linea, char color, unsigned char delay) {
             if (i % TICKS == 0) sched();
                }
     }
+       
+       task_exit();
 }
~~~
*diff de task.c*
~~~
@@ -41,20 +41,22 @@ void sched() {

     if (new) {
-        old->status = READY;  // XXX 🤔?
+        if (old->status == RUNNING) old->status = READY;  
         current = new;
         current->status = RUNNING;
         swtch(&old->stack, &current->stack);
     }
 }
+
+void task_exit(void){
+       current->status = FREE;
+       sched();
+}
~~~

## term

1)*Eliminar la llamada a task_exit() del contador. Explicar cómo se podría invocar automáticamente al terminarse la tarea.*

Siguiendo la idea de los stubs de un thread que se encargan de llamar a la función en sí y luego salir, se puede agregar en task_spawn un bloque 0  para llamar a task_exit cuando se termine de ejecutar la función que corresponda. Es importante destacar que esto NO es un context switch, sino que al terminar de ejecutarse contador() se llamara a ret, popeando del stack la return address. Si antes del bloque inicial de TaskData se agrega la dirección de task_exit entonces, estamos cumpliendo el objetivo: para la función ejecutada ahora es totalmente transparente.

2)[en contador.c se eliminó la llamada a task_exit]

*diff de task.c*
~~~
@@ -21,7 +21,14 @@ void task_spawn(void (*entry)(void)) {
     while (i < MAX_TASK && Tasks[i].status != FREE)
         i++;
 
-    void *stack = stack_alloc(STACK_SIZE) - sizeof(struct TaskData);
+    void *stack = stack_alloc(STACK_SIZE) - sizeof(void*);
+
+       //agrego la retaddr  
+       void (**exit)(void) = stack;
+       *exit = &task_exit;
+       
+       //Hago lugar para el TaskData inicial
+       stack-=sizeof(struct TaskData);
     Tasks[i].stack = stack;
     Tasks[i].status = READY;

~~~
_ BORRARME DESPUES
# BORRAR DESPUES

## halt

1)*¿Cuánta CPU consume QEMU una vez terminan todas las tareas?*
El 100%.

2)*Si la idle task quisiera invocar la instrucción hlt cuando no hay más tareas activas ¿dónde y cómo se debería hacer?*

