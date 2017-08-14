#!/bin/bash -eux

cd `dirname $0`

rm -rf ./sample-book
cp -rf ~/.rbenv/versions/2.4.1/lib/ruby/gems/2.4.0/gems/review-2.3.0/test/sample-book ./
cd sample-book/src
review-pdfmaker config.yml
open book.pdf
