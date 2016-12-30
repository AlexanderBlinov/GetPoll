# GetPoll API

Суть API - работа с поллами (англ. *poll* - голосование), их добавление, удаление и, естественно, сам процесс голосования.

Это будет REST, так как из контекста задачи следует отсутствие каких либо специфических действий, которые было бы трудно представить в виде CRUD операций. Поэтому работать через представления ресурсов очень даже удобно.

API не будет RESTful, так как в нем не будет реализовано *self-descriptive messages*. Поддержка этого пункта займет слишком много времени, в то время как типов данных не так уж много, и будет гораздо быстрее заложить знание о них на клиенте, чем описывать свои vendor-типы. В качестве Content-Type будет использован простой application/json. Изначально были мысли о hal-json, так как в нем заложены стандарты пердставления ресурсов и гипермедиа. Но он показался давольно сложным для текущего варианта веб-сервиса. Хотя стоит отметить, что саму идею API можно гораздо больше развить, и, возможно, тогда было бы круто использовать HAL или свои vendor-типы.

## Интерфейс

1. `HTTP GET /polls` - выдает коллекцию поллов (реализует Read коллекции)

  Пример ответа

    ```
    {
      "data": [
        {
          "creationDateTime": "2016-10-20T19:36:11+03:00",  // Время создания полла по стандарту ISO 8601
          "links": {
            "self": "/polls/2"                              //  URL на конкретный ресурс полла
          },
          "name": "Best superhero movie 2016"               // Имя пола
        },
        {
          "creationDateTime": "2016-10-20T18:05:47+03:00",
          "links": {
            "self": "/polls/1"
          },
          "name": "Who's gonna be next America president?"
        }
      ],
      "links": {
        "next": "/polls?creationDateTime=2016-10-20T18%3A05%3A47%2B03%3A00",  // URL на следующие n поллов, добавленных до указанной даты
        "self": "/polls"
      }
    }
    ```

2. `HTTP GET /polls/{id}` - возвращает полл с указанным id (реализует Read ресурса)

  Пример ответа

    ```
    {
      "author": "geek_boy4000",                         // Username пользователя, создавшего полл
      "creationDateTime": "2016-10-20T18:05:47+03:00",  // Время создания в стандарте ISO 8601
      "description": "Which movie really nailed it?",   // Дополнительно описание
      "links": {
        "givenVote": "/polls/{id}/votes/{vote_id}",     // Гипермедия на голос, отданный пользователем
        "votes": "/polls/{id}/votes"                    // Гипермедия на коллекцию голосов
      },
      "name": "Best superhero movie 2016",              // Имя голосования
      "options": [                                      // Список вариантов ответа в голосовании
        {
          "id": 1,                                      // Идентификатор варианта ответа (достаточно просто порядкового номера)
          "name": "Bats vs Sups",                       // Имя варианта
          "votes": 123                                  // Число пользователей, выбравших этот ответ
        },
        {
          "id": 2,
          "name": "Civil War",
          "votes": 170
        },
        {
          "id": 3,
          "name": "X-Men",
          "votes": 121
        }
      ],
      "totalVotes": 314                                  // Общее число голосовавших
    }
    ```

3. `HTTP POST /polls` - создает полл и добавляет его в коллекцию polls (реализует Create ресурса)

  Пример запроса

    ```
    {
      "author": "trip_lover18",
      "creationDateTime": "2016-10-20T18:05:47+03:00",
      "description": "Please help me to decide what country I should visit next",
      "name": "What's the best place to visit?",
      "options": [
        {
          "id": 1,
          "name": "Norway"
        },
        {
          "id": 2,
          "name": "Iceland"
        }
      ]
    }
    ```

  В случае успеха возвращает `201 Created`, `Location: /polls/someId` в заголовке и представление созданного объекта в теле.

4. `HTTP DELETE /polls/{id}` - удаляет полл (реализует Delete рeсурса)

  В случае успеха вернет `204 No Content`.

  Если пользователь не является автором полла, вернется `403 Forbidden`.

5. `HTTP POST /polls/{id}/votes` - добавляет голос с указаным вариантом ответа (реализует Create рeсурса)

  Пример запроса

    ```
    {
      "optionId": 1,          // Номер выбранного варианта ответа
      "author": "marta2000"   // Username проголосовавшего пользователя
    }
    ```

  В случае успеха возвращает `201 Created`, `Location: /polls/{id}/votes/someId` в заголовке и представление созданного объекта в теле. Если пользователь уже добавлял свой голос в этом полле, то будет возвращен код `200 OK`.

6. `HTTP PUT /polls/{id}/votes/{vote_id}` - изменяет уже сделанный голос (реализует Update рeсурса)

  Пример запроса

    ```
    {
      "optionId": 2,
      "author": "marta2000"
    }
    ```
  В случае, когда пользователь не является автором, вернется `403 Forbidden`.

7. `HTTP DELETE /polls/{id}/votes/{vote_id}` - удаляет голос в конкретном полле (реализует Delete рeсурса)

  В случае успеха вернет `204 No Content`.

  В случае попытки удалить голос пользователем, который не является его автором, вернется `403 Forbidden`.


# CAP

Чаще всего неотемлемым требованием к распределенным систем является возможность работы каждой ноды независимо от состояние системы в целом. Поэтому partition tolerance мы однозначно берем, и речь пойдет о выборе между CP или AP системой.

## CP?

С одной стороны взять consistency для нашей системы выглядит весьма неплохо: запросы всех пользователей на чтение будут возвразать единственный и к тому же реальный результат. Это особенно важно в случае, когда пользователем является автор какого-либо голосавния, и он хочет подвести итоги этого голосования.

С консистентностью не будет ситуации, когда в случае распада системы на сервере, обрабатывающий запросы из США будет один результат, а на сервере в Европе совершенно другой. Ведь тогда то, какую информацию получит автор голосования, зависит от того, на какой сервер будет направлен его запрос на чтение.

Однако сохраняя консистентность на операцию чтения, мы, в случае распада, ограничиваем часть пользователей от возможности делать запросы на запись, т.е. от возмоности голосовать. Причем это может быть очень большая доля людей, голоса которых могли бы полностью изменить результат голосования.

Помимо недоступности голосовать, данное поведение чревато порчей репутации нашей системы у пользователей. Причем как у тех, кто просто голосует, так и у тех, кто эти голосования создает. Для первых нет смысла пользоваться системой, которая не может их голос учесть, а для вторых нет смысла в системе, которая не может гарантировать, что голоса всех желающих будут учтены.

## AP!

Из выше сказанного следует, что доступность операций записи важна не меньше, чем коситентность данных. Поэтому предлагается реализовать веб-сервис на базе eventually-consistent AP системы с использованием Apache Cassandra.

# Реализация

Веб сервис реализован на C++ с использованием [fastcgi-daemon2](https://github.com/lmovsesjan/Fastcgi-Daemon) и [DataStax cpp-driver for Apache Cassandra](https://github.com/datastax/cpp-driver). Для развертывания кластера с Cassandra изспользовался [Cassandra Cluster Manager](https://github.com/pcmanus/ccm). Он позволяет создавать кластер для Cassandra с нежным количеством узлов на одной машине.

[Source code](https://github.com/AlexanderBlinov/GetPoll/tree/master/GetPoll)

## Модель данных

Аналогичным понятию SCHEMA реляционной модели в Cassandra является KEYSPACE. Важной его особенностью является задание класса стратегии, который зависит от количества имеющихся дата-центров, и фактора репликации для каждого дата-центра. Поскольку при помощи ccm локально был создан только один кластер, представляющий собой 1 дата-центр с 3-мя нодами, то был задан самый просто класс стратегии, с реплика-фактором равным 2. Так же у KEYSPACE можно указать такую опцию как durable rights, которая включает логирование операций записи в ноде-координторе в случае, если ее не удается выполнить сразу.

Аналогом таблиц в Cassandra является column family. У column family как и у таблицы есть понятие Primary Key, при этом формирование ключа влияет как на эффективность распределения данных между узлами, так и на эффективность их чтения. Составной primary key формируется из двух частей: partition key и clustering key. Причем именно partition key влияет на место хранения записи в кластере. При этом он сам может быть составным.

Например, при создании column family для хранения голосов стоял вопрос, какой же partition key взять. Ведь очевидно, что в рамках одного и того же голосования может быть сделано довольно большое кол-во голосов. И следовательно, хотелось бы эти данный хранить как можно кучнее, чтобы уменьшить число обращений в нутри кластера. Этого можно достичь, сделав primary key равным id голосования. Тогда все голоса одгошо и того же голосования будут хранится на одном узле. Однако это порождает проблему, когда в одном и том же голосовании сделаны миллионы голосов. Тогда может получится ситуация с неравномерным распределением данных между узлами. Поэтому было разумным добавить дополнительное поле hash_prefix (можно и suffix), которое является хэшом деленым на модуль n от какой-либо другой колонки. Т.о., выбирая n мы можем найти определенный баланс между распределенностью и локальностью данных. В конкретном случае n=2, поскольку учебный кластер состоял из 3 нод.

Важным моментом так же является согласованность данных, а именно как же она может быть достигнута. На этот случай Cassandra позволяет для каждой операции чтения/записи выставить уровень согласованности. В конкретном случае, использовался уровень ONE для операции чтения и записи. Естественно в общем случае это не даст нам строгой согласованности, однако в рамках учебного проекта этого достаточно, так как реплика-фактор у keyspace был выставлен в 2.

В рельном бы случае, будь у нас несколько дата-центров, я бы подумал над LOCAL_QUORUM для записи и EACH_QUORUM на чтение. Это позволит ускорить операцию записи, и при этом у нас будут сильно согласованные данные по чтению.

[CQL script](https://github.com/AlexanderBlinov/GetPoll/blob/master/GetPoll/cql.txt)

## Load testing

[Load Testing 1 - 400 rps](https://overload.yandex.net/5792#tab=test_data&tags=&plot_groups=main&machines=&metrics=&slider_start=1483056585&slider_end=1483056824)

[Load Testing 2 - 500 rps](https://overload.yandex.net/5869#tab=test_data&tags=&plot_groups=main&machines=&metrics=&slider_start=1483100207&slider_end=1483100446)
