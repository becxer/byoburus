
# some more ls aliases
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
alias lr='ls -lR'
alias lt='ls -ltr'

# some other aliases
alias rm='rm -i'
alias du='du -h'
alias df='df -kh'
alias h='history'
alias vi='vim'
alias vm='vim'
alias em='emacs'

# simple http server
alias serve="python -mSimpleHTTPServer"

# Swap name between files
function swap()
{
    local TMPFILE=tmp.$$
    mv $1 $TMPFILE
    mv $2 $1
    mv $TMPFILE $2
}

# Show host information
function ii()
{
    echo -e "You are logged on $HOST"
    echo -e "Additionnal information: " ; uname -a
    echo -e "Users logged on: " ; w -h
    echo -e "Current date : " ; date
    echo -e "Machine stats : " ; uptime
    echo -e "Memory stats : " ; free
}

# show login avartar
# jp2a --background=light --width=50 --colors ~/.bavatar.jpg
# toilet -f mono9 -F gay "Becxer_"

# Set display name
echo -e '\033[1;36m'Concurrent users are............................'\033[0m'
w
echo -e '\033[1;36m'................................................'\033[0m'

