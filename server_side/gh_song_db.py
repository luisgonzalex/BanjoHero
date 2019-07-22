import sqlite3
import time
songs_db = '__HOME__/finalproject/songs_test4.db'


def request_handler(request):

    if request['method'] == 'POST':
        values = request['form']
        song_title = values['title']
        red = values['red']
        green = values['green']
        blue = values['blue']
        conn = sqlite3.connect(songs_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # make cursor into database (allows us to execute commands)
        c.execute('''CREATE TABLE IF NOT EXISTS song_table (red text, green text, blue text, title text);''')  # run a CREATE TABLE command
        c.execute(''' SELECT * FROM song_table WHERE title = ?;''', (song_title,))
        data = c.fetchone()
        if data is None:
            c.execute('''INSERT into song_table VALUES (?,?,?,?);''', (red, green, blue, song_title))
        else:
            c.execute('''UPDATE song_table SET red = ?, green = ?, blue = ?, title = ?;''',(red, green, blue, song_title))
        conn.commit()  # commit commands
        conn.close()  # close connection to database

    elif request['method'] == 'GET':
        values = request['values']
        conn = sqlite3.connect(songs_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # make cursor into database (allows us to execute commands)
        title = values['title']
        things = c.execute('''SELECT * FROM song_table where title = (?);''', (title,)).fetchone()
        outs = ''
        for color in things[:-1]:
            outs += color
            outs += '&'
        return outs[:-1]
