
global.ian_dev = true;

const path = require('path');
const merge = require('webpack-merge');
const common = require('./webpack.common.js');

module.exports = merge(common, {
  devtool: 'inline-source-map',

  devServer: {
    contentBase: path.resolve('@CMAKE_INSTALL_PREFIX@/client')
  }
});
