const { sendWebhookMessage } = require('../utils/webhooks');

const createOrder = async (req, res, next) => {

  const { user_id, product_id, price, amount, tax, total } = req.body.order;


  // you may want to store it in a database, and generate an ID automatically
  const data = {
    order_id: 'a1b2c3d4e5f6g7h8i9', // demo ID
    user_id,
    product_id,
    price,
    amount,
    tax,
    total,
  };


  // after creating the order, it's the time for creating an event
  // you can pass in whatever data you want to send with the event
    try {
      const result = await sendWebhookMessage('order.created', data)
      res.status(201).json({ message: 'Order has been created successfully', result })
    } catch (error) {
      console.log(error);
      res.status(error.status).json({ message: error.message });
    }
};

const createEvent = async (req, res, next) => {

  // now you've received an event from a specific service,
  // and you want to create an event to notify your subscribers
  const { event_type, provider_message, provider_name, status } = req.body.event;


  // you may want to query the order data of `order_id` and send it with the message
  const orderId = req.params.order_id;

  const order = { // ex: order data from a database
    order_id: orderId,
    user_id: 'fc741db0a2968c39d9c2a5cc75b05370',
    product_id: 'b31d032cfdcf47a399990a71e43c5d2a',
    price: '20.99',
    amount: '2',
    tax: '3.8',
    total: '45.78',
  };

  
  // you can form the data object as you like
  const data = {
    event_type,           // ex: 'fulfilled'
    provider_message,     // ex: 'your order may take two days on the way'
    provider_name,        // ex: 'FedEx'
    status,               // ex: 'safe'
    order,                // the order data
  };


  try {
    const result = await sendWebhookMessage(`order.${event_type}`, data)
    res.status(201).json({ message: 'Event has been created successfully', result })
  } catch (error) {
    console.log(error);
    res.status(error.status).json({ message: error.message });
  }
};

const getOrders = (req, res, next) => {
  res.status(200).json({ data: 'Your Orders' });
};

const getOrder = (req, res, next) => {
  res.status(200).json({ data: 'Your Order' });
};

module.exports = {
  createOrder,
  createEvent,
  getOrders,
  getOrder,
};
