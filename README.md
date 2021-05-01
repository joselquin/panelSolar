# PanelSolar 

Pruebas de alimentación con panel solar con ESP32

JLQG(2021) - mar2021

El objeto de este proyecto es colocar un panel solar en las 4 vertientes de una caseta y medir la tensión que resulta del mismo cada 10s durante 48h en cada vertiente. Las mediciones las realiza un ESP32, gracias a su conversor AD y las envía a una base de datos situada en un servidor virtual a través de MQTT.

El servicio MQTT reside tambiérn en el mismo servidor. En el servidor se tiene una aplicación node que recoge los mensajes y los distribuye a la tabla apropiada de la base de datos.

Nota: la versión del código node aquí cargada no es la utilizada en el proyecto real, ya que la aplicación real que estoy usando sirve para varios proyectos; los mensajes tienen diversos tópicos con los que la aplicación node diferencia cada mensaje y hace las consultas adecuadas con SQL.
