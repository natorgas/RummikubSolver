with open('src/Player.cpp', 'r') as f:
    content = f.read()

# Extract the body of find_best_move
start_idx = content.find("bool AIPlayer::find_best_move(")
end_idx = content.find("void AIPlayer::set_hand(")
func_body = content[start_idx:end_idx]

lines = func_body.split('\n')
for i, line in enumerate(lines):
    if "if (unplacedBoardTiles == 0)" in line:
        print(f"L{i}: {line.strip()}")
    elif "if (unplacedBoardTiles > 0)" in line:
        print(f"L{i}: {line.strip()}")
    elif "else {" in line and lines[i-1].strip() == "}":
        print(f"L{i}: {line.strip()}")
