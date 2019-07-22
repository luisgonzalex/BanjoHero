# 6.08 Guitar Hero Server Side Script
#import matplotlib.pyplot as plt
#from scipy.io import wavfile
#from scipy.fftpack import fft,fftfreq
import sqlite3
score_db = '__HOME__/finalproject/score.db'


#samplerate, data = wavfile.read("seeyouagain.wav")

# plt.figure(1)
# plt.plot(data[:4*samplerate]) #plot first 4 seconds
# plt.plot(data[:])

# plt.xlabel('time')
# plt.ylabel('frequency')
# plt.show()

def request_handler(request):
    try:
        if request["method"] == "POST":
            # POST request: chosing a song or posting scores
            user = request["form"]["user"]
            #cap username at 8 characters
            userlen = len(user)
            user = user[0:min(userlen, 5)]
            userlen = len(user)
            while userlen < 4:
                user += " "
                userlen+=1

            score = request["form"]["score"]
            scorelen = len(score)
            while scorelen < 4:
                score += " "
                scorelen+=1


            song = request["form"]["song"]
            #cap song name at 8 characters
            songlen = len(song)
            song = song[0:min(songlen, 11)]
            songlen = len(song)
            while songlen < 11:
                song += " "
                songlen+=1


            percent = request["form"]["percent"]
            perlen = len(percent)
            while perlen < 3:
                percent += " "
                perlen+=1

            # store song and score into a database
            # connect to that database (will create if it doesn't already exist)
            conn = sqlite3.connect(score_db)
            c = conn.cursor()  # make cursor into database (allows us to execute commands)

            c.execute(
                '''CREATE TABLE IF NOT EXISTS userdata (user, song, score, percent); ''')  # Username, song, score, percentage
            c.execute('''INSERT into userdata VALUES (?,?,?,?); ''',
                      (user, song, score, percent))

            # delete everything just in case
            #c.execute('''DELETE FROM userdata;''')
            alltimebest = c.execute(
                '''SELECT * FROM userdata ORDER BY score DESC;''').fetchall()
            conn.commit()  # commit commands
            conn.close()  # close connection to database


            place = ["user", "song", "score", "percent"]


            newalltimebest = "User    Song    Score   % "
            newalltimebest +="--------------------------"
            for scores in alltimebest:
                for data in scores:
                    newalltimebest+=data
                    newalltimebest+="|"
                newalltimebest = newalltimebest[0:-1]

            return newalltimebest
        else:
            # GET request: get song data

            val = request["values"]["val"]

            # extract from databse
            # connect to that database (will create if it doesn't already exist)
            conn = sqlite3.connect(score_db)
            c = conn.cursor()  # make cursor into database (allows us to execute commands)
            if val == "alltime":
                alltimebest = c.execute(
                    '''SELECT * FROM userdata ORDER BY score DESC;''').fetchall()
                conn.commit()  # commit commands
                conn.close()  # close connection to database

                newalltimebest = "User    Song    Score   % "
                newalltimebest +="--------------------------"

                for scores in alltimebest:
                    for data in scores:
                        newalltimebest += data
                        newalltimebest += "|"
                    newalltimebest = newalltimebest[0:-1]

                return newalltimebest
            elif val == "personal":
                song = request["values"]["song"]
                user = request["values"]["user"]

                personalbest = c.execute(
                    '''SELECT * FROM userdata WHERE song = ? AND user = ? ORDER BY score DESC;''',
                    (song, user)).fetchall()

                conn.commit()  # commit commands
                conn.close()  # close connection to database

                newpersonalbest = "User    Song    Score   % "
                newpersonalbest +="--------------------------"
                for scores in personalbest:
                    for data in scores:
                        newpersonalbest += data
                        newpersonalbest += "|"
                    newpersonalbest = newpersonalbest[0:-1]

                return newpersonalbest

            elif val == "song":
                song = request["values"]["song"]
                songbest = c.execute(
                    '''SELECT * FROM userdata WHERE song = ? ORDER BY score DESC;''',
                    (song,)).fetchall()

                conn.commit()  # commit commands
                conn.close()  # close connection to database

                newsongbest = "User    Song    Score   % "
                newsongbest +="--------------------------"
                for scores in songbest:
                    for data in scores:
                        newsongbest += data
                        newsongbest += "|"
                    newsongbest = newsongbest[0:-1]

                return newsongbest
            else:
                raise ValueError
    except:
        return "ask for sm else bruh"
