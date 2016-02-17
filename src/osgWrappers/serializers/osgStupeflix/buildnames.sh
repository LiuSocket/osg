cat * | grep REGISTER | awk '{print $2}' | sed 's/,//g' | sort -u | awk '{print "USE_SERIALIZER_WRAPPER(", $1 , ")"}'
