
// Joshua Shepherd and Ben Scott, milestone 3 task 1

#include "filter.h"
#include "stdio.h"
#define FILTER_INIT_ZERO 0
#define DECIMATE_VAL 10
#define FIR_COEF_COUNT 81
#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 11
#define Z_QUEUE_SIZE 10
#define NUMBER_OF_PLAYERS 10
#define OUTPUT_QUEUE_SIZE 2001
#define IIR_B_COEF_COUNT 11
#define IIR_A_COEF 10
#define X_QUEUE_NAME "xQueue"
#define Y_QUEUE_NAME "yQueue"
#define MOST_RECENT_VALUE (OUTPUT_QUEUE_SIZE - 1)
#define OLDEST_VALUE 0
#define INIT_VALUE 0
#define INDEX_OF_ZERO 1
#define INIT_FLOAT 0.0

// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.

// queues in the filter

static queue_t yQueue;
static queue_t xQueue;
static queue_t zQueue[NUMBER_OF_PLAYERS];
static queue_t outputQueue[NUMBER_OF_PLAYERS];

// fir filter count (every 10 values, run fir filter)
static uint32_t filter_count = INIT_VALUE;

// A nd B coefficients double arrays for 10 bandpas filters
const static double a_coeffs[NUMBER_OF_PLAYERS][IIR_A_COEF] = {
    {-5.9637727070163997e+00, 1.9125339333078237e+01, -4.0341474540744159e+01,
     6.1537466875368793e+01, -7.0019717951472131e+01, 6.0298814235238815e+01,
     -3.8733792862566247e+01, 1.7993533279581037e+01, -5.4979061224867571e+00,
     9.0332828533799470e-01},
    {-4.6377947119071452e+00, 1.3502215749461568e+01, -2.6155952405269744e+01,
     3.8589668330738313e+01, -4.3038990303252589e+01, 3.7812927599537062e+01,
     -2.5113598088113729e+01, 1.2703182701888053e+01, -4.2755083391143351e+00,
     9.0332828533799847e-01},
    {-3.0591317915750929e+00, 8.6417489609637492e+00, -1.4278790253808836e+01,
     2.1302268283304297e+01, -2.2193853972079218e+01, 2.0873499791105438e+01,
     -1.3709764520609388e+01, 8.1303553577931673e+00, -2.8201643879900509e+00,
     9.0332828533800058e-01},
    {-1.4071749185996729e+00, 5.6904141470697454e+00, -5.7374718273676137e+00,
     1.1958028362868870e+01, -8.5435280598354275e+00, 1.1717345583835916e+01,
     -5.5088290876998336e+00, 5.3536787286077381e+00, -1.2972519209655504e+00,
     9.0332828533799436e-01},
    {8.2010906117760285e-01, 5.1673756579268542e+00, 3.2580350909220863e+00,
     1.0392903763919165e+01, 4.8101776408668915e+00, 1.0183724507092466e+01,
     3.1282000712126603e+00, 4.8615933365571715e+00, 7.5604535083144453e-01,
     9.0332828533799381e-01},
    {2.7080869856154473e+00, 7.8319071217995511e+00, 1.2201607990980699e+01,
     1.8651500443681549e+01, 1.8758157568004453e+01, 1.8276088095998919e+01,
     1.1715361303018824e+01, 7.3684394621253020e+00, 2.4965418284511718e+00,
     9.0332828533799758e-01},
    {4.9479835250075892e+00, 1.4691607003177600e+01, 2.9082414772101053e+01,
     4.3179839108869317e+01, 4.8440791644688865e+01, 4.2310703962394321e+01,
     2.7923434247706417e+01, 1.3822186510471003e+01, 4.5614664160654321e+00,
     9.0332828533799903e-01},
    {6.1701893352279864e+00, 2.0127225876810332e+01, 4.2974193398071669e+01,
     6.5958045321253422e+01, 7.5230437667866568e+01, 6.4630411355739838e+01,
     4.1261591079244113e+01, 1.8936128791950530e+01, 5.6881982915180274e+00,
     9.0332828533799781e-01},
    {7.4092912870072469e+00, 2.6857944460290192e+01, 6.1578787811202446e+01,
     9.8258255839887752e+01, 1.1359460153696361e+02, 9.6280452143026722e+01,
     5.9124742025776840e+01, 2.5268527576524427e+01, 6.8305064480743756e+00,
     9.0332828533801002e-01},
    {8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037067e+01,
     1.3928510844056819e+02, 1.6305115418161625e+02, 1.3648147221895792e+02,
     8.0686288623299774e+01, 3.2276361903872122e+01, 7.9045143816244750e+00,
     9.0332828533799692e-01}};

const static double b_coeffs[NUMBER_OF_PLAYERS][IIR_B_COEF_COUNT] = {
    {9.0928548030181028e-10, -0.0000000000000000e+00, -4.5464274015090516e-09,
     -0.0000000000000000e+00, 9.0928548030181033e-09, -0.0000000000000000e+00,
     -9.0928548030181033e-09, -0.0000000000000000e+00, 4.5464274015090516e-09,
     -0.0000000000000000e+00, -9.0928548030181028e-10},
    {9.0928699738701766e-10, 0.0000000000000000e+00, -4.5464349869350879e-09,
     0.0000000000000000e+00, 9.0928699738701758e-09, 0.0000000000000000e+00,
     -9.0928699738701758e-09, 0.0000000000000000e+00, 4.5464349869350879e-09,
     0.0000000000000000e+00, -9.0928699738701766e-10},
    {9.0928651287175419e-10, 0.0000000000000000e+00, -4.5464325643587714e-09,
     0.0000000000000000e+00, 9.0928651287175428e-09, 0.0000000000000000e+00,
     -9.0928651287175428e-09, 0.0000000000000000e+00, 4.5464325643587714e-09,
     0.0000000000000000e+00, -9.0928651287175419e-10},
    {9.0928698790167336e-10, 0.0000000000000000e+00, -4.5464349395083673e-09,
     0.0000000000000000e+00, 9.0928698790167347e-09, 0.0000000000000000e+00,
     -9.0928698790167347e-09, 0.0000000000000000e+00, 4.5464349395083673e-09,
     0.0000000000000000e+00, -9.0928698790167336e-10},
    {9.0928659146360622e-10, 0.0000000000000000e+00, -4.5464329573180310e-09,
     0.0000000000000000e+00, 9.0928659146360620e-09, 0.0000000000000000e+00,
     -9.0928659146360620e-09, 0.0000000000000000e+00, 4.5464329573180310e-09,
     0.0000000000000000e+00, -9.0928659146360622e-10},
    {9.0928682036820422e-10, -0.0000000000000000e+00, -4.5464341018410212e-09,
     -0.0000000000000000e+00, 9.0928682036820424e-09, -0.0000000000000000e+00,
     -9.0928682036820424e-09, -0.0000000000000000e+00, 4.5464341018410212e-09,
     -0.0000000000000000e+00, -9.0928682036820422e-10},
    {9.0928335350797498e-10, -0.0000000000000000e+00, -4.5464167675398755e-09,
     -0.0000000000000000e+00, 9.0928335350797510e-09, -0.0000000000000000e+00,
     -9.0928335350797510e-09, -0.0000000000000000e+00, 4.5464167675398755e-09,
     -0.0000000000000000e+00, -9.0928335350797498e-10},
    {9.0929473327455508e-10, 0.0000000000000000e+00, -4.5464736663727757e-09,
     0.0000000000000000e+00, 9.0929473327455514e-09, 0.0000000000000000e+00,
     -9.0929473327455514e-09, 0.0000000000000000e+00, 4.5464736663727757e-09,
     0.0000000000000000e+00, -9.0929473327455508e-10},
    {9.0926049582391775e-10, 0.0000000000000000e+00, -4.5463024791195884e-09,
     0.0000000000000000e+00, 9.0926049582391769e-09, 0.0000000000000000e+00,
     -9.0926049582391769e-09, 0.0000000000000000e+00, 4.5463024791195884e-09,
     0.0000000000000000e+00, -9.0926049582391775e-10},
    {9.0908263379932385e-10, 0.0000000000000000e+00, -4.5454131689966202e-09,
     0.0000000000000000e+00, 9.0908263379932404e-09, 0.0000000000000000e+00,
     -9.0908263379932404e-09, 0.0000000000000000e+00, 4.5454131689966202e-09,
     0.0000000000000000e+00, -9.0908263379932385e-10}};

// z array queue names
const char *z_queue_names[NUMBER_OF_PLAYERS] = {
    "zQueue1", "zQueue2", "zQueue3", "zQueue4", "zQueue5",
    "zQueue6", "zQueue7", "zQueue8", "zQueue9", "zQueue10"};

const char *output_queue_names[NUMBER_OF_PLAYERS] = {
    "outputQueue1", "outputQueue2", "outputQueue3", "outputQueue4",
    "outputQueue5", "outputQueue6", "outputQueue7", "outputQueue8",
    "outputQueue9", "outputQueue10"};

// fir filter coefficients
const static double fir_coeffs[FIR_COEF_COUNT] = {
    -0.00108036121505118,  -0.00171741140102791,  -0.00226039981429795,
    -0.00256894543122146,  -0.00250214757216280,  -0.00194877622977071,
    -0.000859549337891412, 0.000724941435887817,  0.00265745769829452,
    0.00468433276567558,   0.00646624745486950,   0.00761981932799241,
    0.00777641569433478,   0.00665087381135830,   0.00410990336296212,
    0.000228475246397483,  -0.00467711451947619,  -0.0100484495806539,
    -0.0151212706056432,   -0.0189982512515606,   -0.0207518720783361,
    -0.0195456406038045,   -0.0147579406414637,   -0.00609095696934296,
    0.00635214837045830,   0.0220352861427392,    0.0400106032240048,
    0.0589935555552807,    0.0774851135983332,    0.0939273985433255,
    0.106873841886407,     0.115152309841040,     0.118000000000000,
    0.115152309841040,     0.106873841886407,     0.0939273985433255,
    0.0774851135983332,    0.0589935555552807,    0.0400106032240048,
    0.0220352861427392,    0.00635214837045830,   -0.00609095696934296,
    -0.0147579406414637,   -0.0195456406038045,   -0.0207518720783361,
    -0.0189982512515606,   -0.0151212706056432,   -0.0100484495806539,
    -0.00467711451947619,  0.000228475246397483,  0.00410990336296212,
    0.00665087381135830,   0.00777641569433478,   0.00761981932799241,
    0.00646624745486950,   0.00468433276567558,   0.00265745769829452,
    0.000724941435887817,  -0.000859549337891412, -0.00194877622977071,
    -0.00250214757216280,  -0.00256894543122146,  -0.00226039981429795,
    -0.00171741140102791,  0.00108036121505118,   -0.000465979811965125,
    .0000459198266283717,  0.000415422862698405,  0.000636897946591658,
    0.000727944495641489,  0.000716659352645294,  0.000630690516727105,
    0.000490523965222130};

// arrays for computing power of signals,
// used to determine if shots were made and by which players

static double currentPowerValue[NUMBER_OF_PLAYERS] = {
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT,
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT};
static double normalizedArray[NUMBER_OF_PLAYERS] = {
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT,
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT};
// double oldOutputArrayValue[NUMBER_OF_PLAYERS] = {};

/*********************************************************************************************************
****************************************** Main Filter Functions
**********************************************************************************************************/

// Must call this prior to using any filter functions.
// initializes the queue and fills them with 0s
void filter_init() {
  queue_init(&xQueue, X_QUEUE_SIZE, X_QUEUE_NAME); // init xQueue : length 81
  // loop through the z queue filling it with 0s
  for (uint32_t j = INIT_VALUE; j < X_QUEUE_SIZE; j++)
    queue_overwritePush(&(xQueue), INIT_FLOAT);

  queue_init(&yQueue, Y_QUEUE_SIZE, Y_QUEUE_NAME); // init yQueue : length 11
  // loop through the y queue filling it with 0s
  for (uint32_t j = INIT_VALUE; j < Y_QUEUE_SIZE; j++) {
    queue_overwritePush(&(yQueue), INIT_FLOAT);
  }
  // loop through all the z queues
  for (uint32_t i = INIT_VALUE; i < NUMBER_OF_PLAYERS;
       i++) { // init each of the 10 zQueue : length 10
    queue_init(&zQueue[i], Z_QUEUE_SIZE, z_queue_names[i]);
    // loop through the number of elements in each z queue filling it with 0s
    for (uint32_t j = INIT_VALUE; j < Z_QUEUE_SIZE; j++) {
      queue_overwritePush(&(zQueue[i]), INIT_FLOAT);
    }
  }
  // loop through each of the 1 output queues
  for (uint32_t i = INIT_VALUE; i < NUMBER_OF_PLAYERS;
       i++) { // queue init for each of the 10 queue in outputQueue : length
              // 2001
    queue_init(&outputQueue[i], OUTPUT_QUEUE_SIZE, output_queue_names[i]);
    // loop through the number of elements in each output queue filling it with
    // 0s
    for (uint32_t j = INIT_VALUE; j < OUTPUT_QUEUE_SIZE; j++) {
      queue_overwritePush(&(outputQueue[i]), INIT_FLOAT);
    }
  }
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) { queue_overwritePush(&xQueue, x); }

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t *q, double fillValue) {
  int32_t length = queue_size(q);
  // loop through the queue length pushing the fill value
  for (uint32_t i = INIT_VALUE; i < length; i++) {
    queue_overwritePush(q, fillValue);
  }
}

// anti aliasing filter
// Invokes the FIR-filter. Input is contents of xQueue.
// SOutput is returned and is also pushed on to yQueue.
double filter_firFilter() {
  double fir_output = INIT_VALUE; // init to 0
  // multiply the vlaues in x queue by the fir coeficents to low pass filter
  for (uint32_t i = INIT_VALUE; i < X_QUEUE_SIZE;
       i++) { // multiply the 81 values
    // compute fir filter first
    fir_output += queue_readElementAt(&xQueue, i) *
                  fir_coeffs[(X_QUEUE_SIZE - INDEX_OF_ZERO) - i];
  }
  queue_overwritePush(&yQueue, fir_output);
  return fir_output;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {
  double iir_output;
  double az_Val = INIT_VALUE;
  double by_Val = INIT_VALUE;
  // sum the b coeficents * the values in y queue
  for (uint32_t i = INIT_VALUE; i < Y_QUEUE_SIZE; i++) {
    by_Val += queue_readElementAt(&yQueue, (Y_QUEUE_SIZE - INDEX_OF_ZERO) - i) *
              b_coeffs[filterNumber][i];
  }
  // sum the a coeficents * the values in the z queue
  for (uint32_t i = INIT_VALUE; i < Z_QUEUE_SIZE; i++) {
    az_Val += queue_readElementAt(&zQueue[filterNumber],
                                  (Z_QUEUE_SIZE - INDEX_OF_ZERO) - i) *
              a_coeffs[filterNumber][i];
  }
  iir_output = by_Val - az_Val;
  queue_overwritePush(&outputQueue[filterNumber], iir_output);
  queue_overwritePush(&zQueue[filterNumber], iir_output);

  return iir_output;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.

double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint) {
  if (forceComputeFromScratch) {
    currentPowerValue[filterNumber] = INIT_VALUE;
    // loop through the output que and sum the square of each value
    for (uint32_t i = 1; i < (OUTPUT_QUEUE_SIZE); i++) {
      // square each value of that array and sum them all
      double value = queue_readElementAt(&outputQueue[filterNumber], i);
      currentPowerValue[filterNumber] += value * value;
    }
  } else { // compute just the most recent value
    double valueNew =
        queue_readElementAt(&outputQueue[filterNumber], MOST_RECENT_VALUE);
    double valueOld =
        queue_readElementAt(&outputQueue[filterNumber], OLDEST_VALUE);
    currentPowerValue[filterNumber] = currentPowerValue[filterNumber] +
                                      (valueNew * valueNew) -
                                      (valueOld * valueOld);
  }
  return currentPowerValue[filterNumber];
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {
  return currentPowerValue[filterNumber];
}
// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {
  // loop through copying each power value
  for (uint32_t i = INIT_VALUE; i < NUMBER_OF_PLAYERS; i++) {
    powerValues[i] = currentPowerValue[i];
  }
}

// Using the previously-computed power values that are current stored in
// currentPowerValue[] array, Copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {
  // reset max value index
  *indexOfMaxValue = INIT_VALUE;
  // find the index of the max power value
  for (uint32_t i = INIT_VALUE; i < NUMBER_OF_PLAYERS; i++) {
    // find the max value
    if (currentPowerValue[i] > currentPowerValue[*indexOfMaxValue]) {
      *indexOfMaxValue = i;
    }
    // copy data values
    normalizedArray[i] = currentPowerValue[i];
  }
  // normalize the array
  for (uint32_t i = INIT_VALUE; i < NUMBER_OF_PLAYERS; i++) {
    // divide the normalized array values by the max value
    normalizedArray[i] =
        normalizedArray[i] / currentPowerValue[*indexOfMaxValue];
  }
}

/*********************************************************************************************************
********************************** Verification-assisting functions.
**************************************
********* Test functions access the internal data structures of the filter.c via
*these functions. ********
*********************** These functions are not used by the main filter
*functions. ***********************
**********************************************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() { return fir_coeffs; }

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() { return FIR_COEF_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
  return a_coeffs[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() { return IIR_A_COEF; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
  return b_coeffs[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() { return IIR_B_COEF_COUNT; }

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() { return Y_QUEUE_SIZE; }

// Returns the decimation value.
uint16_t filter_getDecimationValue() { return DECIMATE_VAL; }

// Returns the address of xQueue.
queue_t *filter_getXQueue() { return &xQueue; }

// Returns the address of yQueue.
queue_t *filter_getYQueue() { return &yQueue; }

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
  return &zQueue[filterNumber];
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
  return &outputQueue[filterNumber];
}

uint32_t filter_getZArraySize() { return NUMBER_OF_PLAYERS; }

// void filter_runTest();
