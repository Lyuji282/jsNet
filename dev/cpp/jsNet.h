#include <vector>
#include <tuple>
#include <map>
#include <tgmath.h>

// For easier debugging
// #include "printv.h"

class Layer;
class Neuron;
class Filter;
class NetMath;
class NetUtil;

class Network {
public:
    static std::vector<Network*> netInstances;
    int instanceIndex;
    int iterations=0;
    int validations=0;
    int validationInterval;
    int miniBatchSize;
    int channels;
    float learningRate=0;
    float momentum=0;
    float rmsDecay=0;
    float rho=0;
    float lreluSlope=0;
    float rreluSlope=0;
    float eluAlpha=0;
    bool isTraining;
    float dropout=0;
    double l2=0;
    double l2Error=0;
    double l1=0;
    double l1Error=0;
    float maxNorm=0;
    double maxNormTotal=0;
    double trainingLogging;
    double error=0;
    double validationError=0;
    double lastValidationError;
    bool stoppedEarly=false;
    int earlyStoppingType=0;
    double earlyStoppingThreshold=0;
    double earlyStoppingBestError=0;
    int earlyStoppingPatience=0;
    int earlyStoppingPatienceCounter=0;
    float earlyStoppingPercent=0;
    std::vector<Layer*> layers;
    std::vector<std::tuple<std::vector<double>, std::vector<double> > > trainingData;
    std::vector<std::tuple<std::vector<double>, std::vector<double> > > validationData;
    std::vector<std::tuple<std::vector<double>, std::vector<double> > > testData;
    std::map<std::string, float> weightsConfig;
    double (*activation)(double, bool, Neuron*);
    double (*costFunction)(std::vector<double> calculated, std::vector<double> desired);
    std::vector<double> (*weightInitFn)(int netInstance, int layerIndex, int size);

    std::vector<std::vector<int>> trainingConfusionMatrix;
    std::vector<std::vector<int>> testConfusionMatrix;
    std::vector<std::vector<int>> validationConfusionMatrix;

    bool collectErrors=false;
    std::vector<double> collectedTrainingErrors;
    std::vector<double> collectedValidationErrors;
    std::vector<double> collectedTestErrors;

    int updateFnIndex;

    Network () {}

    ~Network ();

    static int newNetwork(void);

    static void deleteNetwork(void);

    static void deleteNetwork(int index);

    static Network* getInstance(int i);

    void joinLayers();

    std::vector<double> forward (std::vector<double> input);

    void backward (void);

    void train (int iterations, int startIndex);

    double validate (void);

    bool checkEarlyStopping (void);

    double test (int iterations, int startIndex);

    void resetDeltaWeights (void);

    void applyDeltaWeights (void);

    void restoreValidation (void);

};


class Layer {
public:
    int netInstance;
    std::string type;
    int size;
    int fanIn;
    int fanOut;
    int channels;
    int filterSize;
    int stride;
    int zeroPadding;
    int inMapValuesCount;
    int inZPMapValuesCount;
    int outMapSize;
    int prevLayerOutWidth;
    bool hasActivation;
    bool softmax=false;
    std::vector<Neuron*> neurons;
    std::vector<Filter*> filters;
    std::vector<std::vector<std::vector<std::vector<int> > > > indeces;
    std::vector<std::vector<std::vector<double> > > errors;
    std::vector<std::vector<std::vector<double> > > activations;
    std::vector<double> deltaBiases;
    std::vector<double> validationBiases;

    std::vector<std::vector<double> > weights; // FC
    std::vector<std::vector<double> > validationWeights; // FC
    std::vector<std::vector<std::vector<std::vector<double> > > > filterWeights;
    std::vector<std::vector<std::vector<std::vector<double> > > > validationFilterWeights;

    std::vector<std::vector<double> > deltaWeights; // FC
    std::vector<std::vector<std::vector<std::vector<double> > > > filterDeltaWeights;

    std::vector<double> biases; // FC
    std::vector<double> sums; // FC
    std::vector<double> errs; // FC
    std::vector<double> actvns; // FC

    Layer* nextLayer;
    Layer* prevLayer;
    double (*activation)(double, bool, Neuron*);
    double (*activationC)(double, bool, Filter*);
    double (*activationP)(double, bool, Network*);

    Layer (int netI, int s) {};

    virtual ~Layer(void) {};

    virtual void assignNext (Layer* l) = 0;

    virtual void assignPrev (Layer* l) = 0;

    virtual void init (int layerIndex) = 0;

    virtual void forward (void) = 0;

    virtual void backward (bool lastLayer) = 0;

    virtual void applyDeltaWeights (void) = 0;

    virtual void resetDeltaWeights (void) = 0;

    virtual void backUpValidation (void) = 0;

    virtual void restoreValidation (void) = 0;

};

class FCLayer : public Layer {
public:

    FCLayer (int netI, int s);

    ~FCLayer (void);

    void assignNext (Layer* l);

    void assignPrev (Layer* l);

    void init (int layerIndex);

    void forward (void);

    void backward (bool lastLayer);

    void applyDeltaWeights (void);

    void resetDeltaWeights (void);

    void backUpValidation (void);

    void restoreValidation (void);
};

class ConvLayer : public Layer {
public:

    ConvLayer (int netI, int s);

    ~ConvLayer (void);

    void assignNext (Layer* l);

    void assignPrev (Layer* l);

    void init (int layerIndex);

    void forward (void);

    void backward (bool lastLayer);

    void backward (void) {
        backward(false);
    };

    void applyDeltaWeights (void);

    void resetDeltaWeights (void);

    void backUpValidation (void);

    void restoreValidation (void);

};

class PoolLayer : public Layer {
public:

    PoolLayer (int netI, int s);

    ~PoolLayer (void);

    void assignNext (Layer* l);

    void assignPrev (Layer* l);

    void init (int layerIndex);

    void forward (void);

    void backward (bool lastLayer);

    void backward (void) {
        backward(false);
    };

    void applyDeltaWeights (void) {};

    void resetDeltaWeights (void) {};

    void backUpValidation (void) {};

    void restoreValidation (void) {};
};


class Neuron {
    public:
        std::vector<double> weightGain;
        std::vector<double> weightsCache;
        std::vector<double> adadeltaCache;
        double lreluSlope;
        double rreluSlope;
        double derivative;
        double eluAlpha;
        double biasGain;
        double adadeltaBiasCache;
        double biasCache;
        double m;
        double v;
        bool dropped;

        Neuron(void) {}

        void init (int netInstance, int weightsCount);
};

class Filter {
public:
    std::vector<std::vector<std::vector<double> > > weightGain;
    std::vector<std::vector<std::vector<double> > > weightsCache;
    std::vector<std::vector<std::vector<double> > > adadeltaCache;
    std::vector<std::vector<double> > sumMap;
    std::vector<std::vector<bool> > dropoutMap;
    double lreluSlope;
    double rreluSlope;
    double derivative;
    double activation;
    double eluAlpha;
    double biasGain;
    double adadeltaBiasCache;
    double biasCache;
    double m;
    double v;
    bool dropped;

    Filter (void) {}

    void init (int netInstance, int channels, int filterSize);
};


class NetMath {
public:
    template <class T>
    static double sigmoid(double value, bool prime, T* neuron);

    template <class T>
    static double tanh(double value, bool prime, T* neuron);

    template <class T>
    static double lecuntanh(double value, bool prime, T* neuron);

    template <class T>
    static double relu(double value, bool prime, T* neuron);

    template <class T>
    static double lrelu(double value, bool prime, T* neuron);

    template <class T>
    static double rrelu(double value, bool prime, T* neuron);

    template <class T>
    static double elu(double value, bool prime, T* neuron);

    static double meansquarederror (std::vector<double> calculated, std::vector<double> desired);

    static double rootmeansquarederror (std::vector<double> calculated, std::vector<double> desired);

    static double crossentropy (std::vector<double> target, std::vector<double> output);

    static double vanillasgd (int netInstance, double value, double deltaValue);

    static double gain(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double gain(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static double adagrad(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double adagrad(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static double rmsprop(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double rmsprop(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static double adam(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double adam(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static double adadelta(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double adadelta(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static double momentum(int netInstance, double value, double deltaValue, Neuron* neuron, int weightIndex);

    static double momentum(int netInstance, double value, double deltaValue, Filter* filter, int c, int r, int v);

    static std::vector<double> uniform (int netInstance, int layerIndex, int size);

    static std::vector<double> gaussian (int netInstance, int layerIndex, int size);

    static std::vector<double> lecununiform (int netInstance, int layerIndex, int size);

    static std::vector<double> lecunnormal (int netInstance, int layerIndex, int size);

    static std::vector<double> xavieruniform (int netInstance, int layerIndex, int size);

    static std::vector<double> xaviernormal (int netInstance, int layerIndex, int size);

    static std::vector<double> softmax (std::vector<double> values);

    static void maxPool (PoolLayer* layer, int channels);

    static void maxNorm(int netInstance);

    static double sech (double value);
};

class NetUtil {
public:

    static void shuffle (std::vector<std::tuple<std::vector<double>, std::vector<double> > > &values);

    static std::vector<std::vector<double> > addZeroPadding (std::vector<std::vector<double> > map, int zP);

    static std::vector<std::vector<double> > convolve(std::vector<std::vector<std::vector<double> > > input, int zP,
        std::vector<std::vector<std::vector<double> > > weights, int channels, int stride, double bias);

    static std::vector<std::vector<double> > arrayToMap (std::vector<double> array, int size);

    static std::vector<std::vector<std::vector<double> > > arrayToVolume (std::vector<double> array, int channels);

    template <class T>
    static std::vector<std::vector<std::vector<T> > > createVolume (int depth, int rows, int columns, T value);

    static std::vector<std::vector<double> > buildConvErrorMap (int paddedLength, Layer* nextLayer, int filterI);

    static void buildConvDWeights (ConvLayer* layer);

    static std::vector<double> getActivations (Layer* layer, int mapStartI, int mapSize);

};
