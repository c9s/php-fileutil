#!/bin/bash
set -e
phpunit
extunit --phpunit --repeat 500
