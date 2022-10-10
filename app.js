const express = require('express');

// load environment variables
require('dotenv').config();

const orderRoutes = require('./routes/order');

const app = express();


// your app global middlewares
app.use(express.json());
app.use(express.urlencoded({ extended: false }));


// routing your http requests
app.use('/api', orderRoutes);


// `Not Found` request handler
app.use((req, res, next) => {
  const error = new Error('Not Found');
  error.status = 404;
  throw error;
});


// thrown erros handler
app.use((error, req, res, next) => {
  res.status(error.status || 500);
  res.json({ message: error.message || 'Internal Server Error' });
});


// bootstrap your server
app.listen(process.env.PORT, () => console.log('Server running on port ' + process.env.PORT));
