#!/bin/bash -e

cd `dirname $0`

if [ -z $1 ] ; then
  echo 'ファイル名を指定してください'
  exit 1;
fi

cd articles
FILENAME="$1".md
if [ ! -f $FILENAME ] ; then
  echo $FILENAME"は存在しません"
  exit 1;
fi

set -ux
md2review $FILENAME > $1.re
sed -i -e "s/emlist\[\]\[\(.*\)\]/list[\1][\1]/" $1.re
sed -i -e "s/<br>/@<br>{}/" $1.re
review-pdfmaker config.yml
open tbf03.pdf
