shopt -s histappend
PROMPT_COMMAND='history -a'

# Увеличить размер истории
HISTSIZE=5000
HISTFILESIZE=10000

# Игнорировать повторяющиеся команды
HISTCONTROL=ignoredups
