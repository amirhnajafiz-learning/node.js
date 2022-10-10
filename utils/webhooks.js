const fetch = require('node-fetch');

// your webhooks utils

const sendWebhookMessage = async (event, data) => {
  var url = new URL(
    `https://www.hostedhooks.com/api/v1/apps/${process.env.APP_UUID}/messages`
  );

  // webhook message
  var messagePayload = JSON.stringify({
    data: {
      order: data, // order data
    },
    version: '1.0',
    event_type: event, // ex: 'order.fullfilled'
  });

  var requestOptions = {
    method: 'POST',
    headers: {
      Authorization: `Bearer ${process.env.HOSTEDHOOKS_API_KEY}`,
      'Content-Type': 'application/json',
    },
    body: messagePayload,
    redirect: 'follow',
  };

  try {
    const result = await fetch(url, requestOptions)
    return result.text()
  } catch (error) {
    throw error
  }

};

module.exports = {
  sendWebhookMessage,
};
