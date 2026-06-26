@echo off

cutechess-cli -engine name=Crystall cmd=./crystall.exe ^
              -engine name=Stockfish cmd=./stockfish.exe option.UCI_LimitStrength=true option.UCI_Elo=1800 ^
              -each tc=60+0.1 proto=uci ^
              -games 1000 ^
              -sprt alpha=0.05 beta=0.05 elo0=0 elo1=10 ^
              -concurrency 6 ^
              -repeat ^
              -pgnout games.pgn