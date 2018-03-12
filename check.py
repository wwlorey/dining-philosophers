import os
import re

firstLines = ["Roses are red, violets are blue,"]

midLines = ["Use one of these poems",
"The center of my world",
"It's you and me together",
"The way that you kiss me",
"I've never experienced",
"You will always be",
"Call it what you want to, but",
"I just want you to know that",
"I'm seeing a future filled with",
"What I feel in my heart",
"The depth of my love?",
"You don't always smell good, but",
"We have nothing in common, so",
"Clinging vines are almost as bad as",
"You have a great sense of humor that has",
"I'm telling you I love you, so",
"We go together like",
"I want to have sex now,",
"I already said I was sorry, so",
"Sure there are others,",
"You are stupid,"]

lastLines = ["as a love message from you",
"begins and ends with you.",
"a lifetime of love, just for two.",
"....Whew!",
"a love so true.",
"my special Boo.",
"I need to be with you.",
"I think the world of you.",
"me and you.",
"is wonderful and new.",
"If you only knew.......",
"I still love you.",
"baby we're through.",
"super glue.",
"me hooked on you.",
"what are you gonna do?",
"my clothes match with my shoes.",
"don't you think I've paid my dues?",
"why continue to stew?",
"not many more than two.",
"he/she is using you."]

files = [name for name in os.listdir() if re.search('outFile.*', name)]
print("The following files were found:\n", str(files))

def checkOutFiles():
    for fName in files:
        f = open(fName, 'r')
        lineCount = 0

        for line in f:
            line = line.strip()
            if lineCount == 0:
                if not line in firstLines:
                    return False
            elif lineCount == 1:
                if not line in midLines:
                    return False
            elif lineCount == 2:
                if not line in lastLines:
                    return False
            elif lineCount == 3:
                lineCount = -1 # Yes, I know my coding style is great ;)

            lineCount += 1

    return True
        
if checkOutFiles():
    print("All outFiles are correct")
else:
    print("There is an issue with some outFile")


