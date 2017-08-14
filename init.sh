#!/bin/bash -eux

cd `dirname $0`

rbenv install -s 2.4.1
rbenv rehash
gem install bundler
bundle install
