<?php
    echo "Starting...\n";
    $context = new ZMQContext();

    $sub_socket = new ZMQSocket($context, ZMQ::SOCKET_SUB);
    $sub_socket->setSockOpt(ZMQ::SOCKOPT_SUBSCRIBE, "SMS");
    $sub_socket->connect("tcp://localhost:5563");

    while (1) {
        #$address->recv($sub_socket);
        #$phone->recv($sub_socket);
        #$obj = json_decode($sub_socket->recv());
        #echo $obj['phone_number']." ".$obj['message']."\n";
        $json_data = $sub_socket->recv();

        #var_dump($sub_socket->recv());
        var_dump(json_decode($json_data));
    }
?>