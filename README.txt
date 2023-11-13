1. Los integrantes del grupo son los siguientes:
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero

2. Mejoras de la práctica:
Decidimos hacer el initMB más eficiente.

Hemos mejorado la función mi_dir para que también imprima la información de los ficheros
y además le hemos añadido colorines para distinguirlos más fácilmente. Para ello,
hemos utilizado una función auxiliar llamada aux_informacion_inodo_dir.

En cuanto a los programas, hemos añadido el mi_touch.c, mi_rmdir.c y mi_rm_r.c.

También hemos añadido una mini caché para escritura y para lectura, para ir guardando las últimas entradas
de los inodos visitados más recientemente.
Además, a la hora de substituir alguna entrada en caso de que
se llene del todo, hemos utilizado un algoritmo RLU para echar la entrada más viejuna.

3. Observaciones generales:
Hemos modificado el scripte2.sh para mostrar la funcionalidad de mi_rm_r.c.
Hemos añadido una estructura para tratar la cache de lecturas y escrituras.